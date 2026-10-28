#!/usr/bin/env python3
"""Capture lifetime, publication, cancellation and preservation regressions."""

import gc
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import textwrap
import time
import unittest
from unittest import mock

import proof_artifacts as artifacts


class CaptureArtifactTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)

    @staticmethod
    @artifacts.capture_artifacts
    def capture(parent, *, _artifacts):
        directory = _artifacts.directory(parent)
        path = directory / 'linked.elf'
        path.write_bytes(b'private synthetic evidence')
        def current():
            if path.read_bytes() != b'private synthetic evidence':
                raise ValueError('evidence changed')
            return True
        current.pointer_scalar_capture = object()
        return path, _artifacts.hold(current)

    def test_retained_or_copied_checker_owns_files_and_rejects_mutation(self):
        path, current = self.capture(self.root)
        copy = current
        del current
        gc.collect()
        self.assertTrue(path.exists())
        self.assertTrue(copy())
        self.assertIs(copy.pointer_scalar_capture, copy.__wrapped__.pointer_scalar_capture)
        path.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'evidence changed'):
            copy()
        del copy
        gc.collect()
        self.assertFalse(path.parent.exists())

    def test_explicit_release_refuses_further_checks(self):
        lease = artifacts.CaptureArtifacts()
        path = lease.directory(self.root)
        current = lease.hold(lambda: True)
        lease.close()
        self.assertFalse(path.exists())
        with self.assertRaisesRegex(RuntimeError, 'released'):
            current()
        lease.close()

    def test_failure_cleans_even_with_retained_exception_traceback(self):
        @artifacts.capture_artifacts
        def fail(*, _artifacts):
            path = _artifacts.directory(self.root)
            (path / 'partial.elf').write_bytes(b'partial')
            raise KeyboardInterrupt('cancelled')
        try:
            fail()
        except KeyboardInterrupt as error:
            saved = error
        self.assertIsNotNone(saved.__traceback__)
        self.assertEqual(list(self.root.iterdir()), [])

    def test_operation_keeps_checks_live_until_publication_then_closes_cycles(self):
        world = {}
        with artifacts.operation():
            path, current = self.capture(self.root)
            world['current'] = current
            world['self'] = world
            self.assertTrue(current())
            (path.parent / 'publication-receipt.json').write_text('{}')
            self.assertTrue(current())
        # Keep the cycle and callback alive: explicit cleanup must not need GC.
        self.assertFalse(path.parent.exists())
        with self.assertRaisesRegex(RuntimeError, 'released'):
            world['current']()

    def test_nested_operation_does_not_release_outer_witness(self):
        with artifacts.operation():
            path, current = self.capture(self.root)
            with artifacts.operation():
                inner_path, inner = self.capture(self.root)
            self.assertTrue(current())
            self.assertTrue(inner())
        self.assertFalse(path.parent.exists())
        self.assertFalse(inner_path.parent.exists())

    def test_repeated_whole_image_captures_do_not_accumulate(self):
        unrelated = self.root / 'capture-user-kept'
        unrelated.mkdir()
        (unrelated / 'notes.txt').write_text('keep')
        for _ in range(12):
            with artifacts.operation():
                path, current = self.capture(self.root)
                with (path.parent / 'full-image.elf').open('wb') as image:
                    image.truncate(32 * 1024 * 1024)
                self.assertTrue(current())
                self.assertEqual(len(list(self.root.glob('capture-*'))), 2)
            self.assertEqual(list(self.root.iterdir()), [unrelated])
        self.assertEqual((unrelated / 'notes.txt').read_text(), 'keep')

    def test_nested_failure_releases_all_operation_files(self):
        with self.assertRaisesRegex(ValueError, 'final refusal'):
            with artifacts.operation():
                first = self.capture(self.root)
                second = self.capture(self.root)
                self.assertTrue(first[1]() and second[1]())
                raise ValueError('final refusal')
        self.assertEqual(list(self.root.iterdir()), [])

    def test_helper_changes_refuse_current_check(self):
        path, current = self.capture(self.root)
        with mock.patch.object(artifacts, '_LOADED', b'changed'):
            with self.assertRaisesRegex(RuntimeError, 'implementation changed'):
                current()
        self.assertTrue(current())

    def test_cleanup_attempts_every_capture_after_an_io_error(self):
        with mock.patch.object(artifacts.shutil, 'rmtree', side_effect=[OSError('unavailable'), None]) as remove:
            with self.assertRaisesRegex(OSError, 'unavailable'):
                artifacts._cleanup([Path('one'), Path('two')])
            self.assertEqual(remove.call_count, 2)

    def test_cli_restores_signal_handler(self):
        previous = signal.getsignal(signal.SIGTERM)
        self.assertEqual(artifacts.run_cli(lambda value: value, 17), 17)
        self.assertEqual(signal.getsignal(signal.SIGTERM), previous)

    def test_sigterm_reaps_live_child_before_removing_capture(self):
        program = textwrap.dedent('''
            import os, pathlib, sys
            import proof_artifacts as a
            import permute_batch as batch
            root=pathlib.Path(sys.argv[1])
            @a.capture_artifacts
            def capture(*, _artifacts):
                directory=_artifacts.directory(root)
                child = "import os,pathlib,sys,time; p=pathlib.Path(sys.argv[1]); (p/'child.pid').write_text(str(os.getpid())); print('ready',flush=True); time.sleep(120)"
                batch.bounded_capture([sys.executable,'-c',child,str(directory)],None,cap=120)
            a.run_cli(capture)
        ''')
        env = dict(os.environ, PYTHONPATH=str(Path(__file__).resolve().parent))
        process = subprocess.Popen([sys.executable, '-c', program, str(self.root)], env=env,
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        child_pid = None
        try:
            deadline = time.monotonic() + 15
            while time.monotonic() < deadline:
                files = list(self.root.glob('capture-*/child.pid'))
                if files:
                    child_pid = int(files[0].read_text())
                    break
                if process.poll() is not None:
                    self.fail('capture process exited early: ' + process.communicate()[0])
                time.sleep(0.02)
            self.assertIsNotNone(child_pid, 'child did not start')
            process.send_signal(signal.SIGTERM)
            output = process.communicate(timeout=25)[0]
            self.assertEqual(process.returncode, 128 + signal.SIGTERM, output)
            self.assertEqual(list(self.root.iterdir()), [])
            with self.assertRaises(ProcessLookupError):
                os.kill(child_pid, 0)
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()
            if child_pid is not None:
                try:
                    os.kill(child_pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass


class OwnedProcessTests(unittest.TestCase):
    setUp = CaptureArtifactTests.setUp
    # Only the additional controls; the base tests already run in their class.
    def test_owned_runner_preserves_output_input_and_check(self):
        command = [sys.executable, '-c',
                   "import sys;print(sys.stdin.read().upper());print('error',file=sys.stderr);sys.exit(7)"]
        result = artifacts.run_owned(command, input='hello', capture_output=True, text=True)
        self.assertEqual((result.returncode,result.stdout,result.stderr),(7,'HELLO\n','error\n'))
        with self.assertRaises(subprocess.CalledProcessError) as raised:
            artifacts.run_owned(command, input=b'x', capture_output=True, check=True)
        self.assertEqual(raised.exception.output,b'X\n')

    def test_timeout_drains_partial_output_and_escalates(self):
        program = "import os,signal,time;signal.signal(signal.SIGTERM,signal.SIG_IGN);print(os.getpid(),flush=True);print('partial',flush=True);time.sleep(120)"
        with mock.patch.object(artifacts,'_TERM_GRACE',0.1):
            with self.assertRaises(subprocess.TimeoutExpired) as raised:
                artifacts.run_owned([sys.executable,'-c',program],text=True,capture_output=True,timeout=0.2)
        self.assertIn('partial',raised.exception.output)
        pid = int(raised.exception.output.splitlines()[0])
        with self.assertRaises(ProcessLookupError):
            os.kill(pid,0)

    def run_cancellation_chain(self, workers, cancel_signal=signal.SIGTERM):
        child = textwrap.dedent("""
            import os,pathlib,sys
            import proof_artifacts as a
            import permute_batch as b
            root=pathlib.Path(sys.argv[1]); number=sys.argv[2]
            @a.capture_artifacts
            def capture(*, _artifacts):
                directory=_artifacts.directory(root/'captures')
                program="import os,pathlib,sys,time;pathlib.Path(sys.argv[1]).write_text(str(os.getpid()));time.sleep(120)"
                b.bounded_capture([sys.executable,'-c',program,str(root/('started-'+number))],None,cap=120)
            a.run_cli(capture)
        """)
        parent = textwrap.dedent("""
            import concurrent.futures,pathlib,sys
            import proof_artifacts as a
            root=pathlib.Path(sys.argv[1]);child=sys.argv[2];workers=int(sys.argv[3])
            def main():
                with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
                    futures=[pool.submit(a.run_owned,[sys.executable,'-c',child,str(root),str(i)],capture_output=True,text=True) for i in range(workers+4)]
                    for future in futures: future.result()
            a.run_cli(main)
        """)
        env=dict(os.environ,PYTHONPATH=str(Path(__file__).resolve().parent))
        process=subprocess.Popen([sys.executable,'-c',parent,str(self.root),child,str(workers)],
                                 env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        pids=[]
        try:
            deadline=time.monotonic()+15
            while time.monotonic()<deadline:
                files=list(self.root.glob('started-*'))
                if len(files)==workers and all(p.read_text() for p in files):
                    pids=[int(p.read_text()) for p in files];break
                if process.poll() is not None:
                    self.fail('parent exited early: '+process.communicate()[0])
                time.sleep(0.02)
            self.assertEqual(len(pids),workers)
            process.send_signal(cancel_signal)
            output=process.communicate(timeout=35)[0]
            if cancel_signal == signal.SIGINT:
                self.assertIn(process.returncode,(-signal.SIGINT,128+signal.SIGINT),output)
            else:
                self.assertEqual(process.returncode,128+cancel_signal,output)
            self.assertEqual(len(list(self.root.glob('started-*'))),workers,
                             'queued work launched after cancellation')
            self.assertEqual(list(self.root.glob('captures/capture-*')),[])
            for pid in pids:
                with self.assertRaises(ProcessLookupError): os.kill(pid,0)
        finally:
            if process.poll() is None:
                process.kill();process.communicate()
            for pid in pids:
                try: os.kill(pid,signal.SIGKILL)
                except ProcessLookupError: pass

    def test_nested_python_cli_cancellation_reaps_detached_compiler(self):
        self.run_cancellation_chain(1)

    def test_threaded_cancellation_stops_active_and_refuses_queued_launches(self):
        self.run_cancellation_chain(2)

    def test_threaded_ctrl_c_stops_active_and_refuses_queued_launches(self):
        self.run_cancellation_chain(2, signal.SIGINT)

    def test_permission_error_does_not_abort_cancellation_broadcast(self):
        processes = {mock.Mock(pid=101),mock.Mock(pid=102)}
        with mock.patch.object(artifacts,'_PROCESSES',processes), \
                mock.patch.object(artifacts.os,'killpg',side_effect=[PermissionError(),None]) as kill:
            artifacts._signal_owned(signal.SIGTERM)
        self.assertEqual(kill.call_count,2)
        self.assertEqual({call.args[0] for call in kill.call_args_list},{101,102})

    def test_timeout_sweeps_silent_descendant_after_leader_exits(self):
        ready = self.root/'silent.pid'
        descendant = "import os,pathlib,signal,sys,time;signal.signal(signal.SIGTERM,signal.SIG_IGN);pathlib.Path(sys.argv[1]).write_text(str(os.getpid()));time.sleep(120)"
        leader = "import subprocess,sys,time;subprocess.Popen([sys.executable,'-c',sys.argv[1],sys.argv[2]],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL);time.sleep(120)"
        pid = None
        try:
            with mock.patch.object(artifacts,'_TERM_GRACE',0.1):
                with self.assertRaises(subprocess.TimeoutExpired):
                    artifacts.run_owned([sys.executable,'-c',leader,descendant,str(ready)],capture_output=True,timeout=1)
            self.assertTrue(ready.exists(),'silent descendant did not start')
            pid = int(ready.read_text())
            deadline = time.monotonic()+3
            while time.monotonic()<deadline:
                try: os.kill(pid,0)
                except ProcessLookupError: break
                time.sleep(0.02)
            else: self.fail('silent descendant survived leader teardown')
        finally:
            if pid is None and ready.exists(): pid=int(ready.read_text())
            if pid is not None:
                try: os.kill(pid,signal.SIGKILL)
                except ProcessLookupError: pass

    def test_failed_leader_sweeps_silent_descendant_with_or_without_check(self):
        for check in (False,True):
            with self.subTest(check=check):
                ready = self.root/('failed-'+str(check)+'.pid')
                descendant = "import os,pathlib,signal,sys,time;signal.signal(signal.SIGTERM,signal.SIG_IGN);pathlib.Path(sys.argv[1]).write_text(str(os.getpid()));time.sleep(120)"
                leader = "import pathlib,subprocess,sys,time;subprocess.Popen([sys.executable,'-c',sys.argv[1],sys.argv[2]],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL);p=pathlib.Path(sys.argv[2]);\nwhile not p.exists(): time.sleep(0.01)\nprint('original output');print('original error',file=sys.stderr);sys.exit(7)"
                pid = None
                try:
                    if check:
                        with self.assertRaises(subprocess.CalledProcessError) as raised:
                            artifacts.run_owned([sys.executable,'-c',leader,descendant,str(ready)],capture_output=True,text=True,check=True,timeout=10)
                        result=raised.exception
                    else:
                        result=artifacts.run_owned([sys.executable,'-c',leader,descendant,str(ready)],capture_output=True,text=True,timeout=10)
                    self.assertEqual((result.returncode,result.stdout,result.stderr),
                                     (7,'original output\n','original error\n'))
                    pid=int(ready.read_text())
                    deadline=time.monotonic()+3
                    while time.monotonic()<deadline:
                        try: os.kill(pid,0)
                        except ProcessLookupError: break
                        time.sleep(0.02)
                    else: self.fail('silent descendant survived failed leader')
                finally:
                    if pid is None and ready.exists(): pid=int(ready.read_text())
                    if pid is not None:
                        try: os.kill(pid,signal.SIGKILL)
                        except ProcessLookupError: pass

    def test_cli_preserves_custom_signal_handlers(self):
        previous = {s:signal.getsignal(s) for s in (signal.SIGINT,signal.SIGTERM)}
        def custom(signum,frame): pass
        try:
            for signum in previous: signal.signal(signum,custom)
            artifacts.run_cli(lambda: self.assertTrue(all(signal.getsignal(s) is custom for s in previous)))
            self.assertTrue(all(signal.getsignal(s) is custom for s in previous))
        finally:
            for signum,handler in previous.items(): signal.signal(signum,handler)


if __name__ == '__main__':
    unittest.main()
