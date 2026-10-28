"""Own private proof files until their final freshness/publication check.

CLI operations close all captures explicitly, including cyclic witnesses.
Library checkers retain their captures until released; failed factories clean
immediately. SIGTERM and Ctrl-C become exceptions so bounded subprocess
runners can stop their children before capture cleanup. SIGKILL and crashes
cannot guarantee cleanup: do not guess that leftover directories are idle.
"""

from __future__ import annotations

from contextlib import ExitStack, contextmanager
from contextvars import ContextVar
import functools
import hashlib
import os
import subprocess
from pathlib import Path
import shutil
import signal
import tempfile
import threading
import weakref


_SOURCE = Path(__file__)
_LOADED = hashlib.sha256(_SOURCE.read_bytes()).digest()
_OPERATION = ContextVar('proof_artifact_operation', default=None)


def _cleanup(directories):
    errors = []
    for path in directories:
        try:
            shutil.rmtree(path)
        except FileNotFoundError:
            pass
        except OSError as error:
            errors.append(error)
    directories.clear()
    if errors:
        raise errors[0]


class CaptureArtifacts:
    """One factory's files, retained by its returned freshness checker."""

    def __init__(self):
        self._directories = []
        self._closed = False
        self._finalizer = weakref.finalize(self, _cleanup, self._directories)
        operation = _OPERATION.get()
        if operation is not None:
            operation.append(self)

    def require_open(self):
        if self._closed:
            raise RuntimeError('proof capture has been released')
        if hashlib.sha256(_SOURCE.read_bytes()).digest() != _LOADED:
            raise RuntimeError('loaded proof artifact implementation changed')

    def directory(self, parent):
        self.require_open()
        parent = Path(parent)
        parent.mkdir(parents=True, exist_ok=True)
        path = Path(tempfile.mkdtemp(prefix='capture-', dir=parent))
        self._directories.append(path)
        return path

    def hold(self, check):
        @functools.wraps(check)
        def current(*args, **kwargs):
            self.require_open()
            return check(*args, **kwargs)
        return current

    def close(self):
        self._closed = True
        self._finalizer()


def capture_artifacts(factory):
    """Clean failed captures even when an exception retains their frames."""
    @functools.wraps(factory)
    def capture(*args, **kwargs):
        artifacts = CaptureArtifacts()
        try:
            return factory(*args, _artifacts=artifacts, **kwargs)
        except BaseException:
            artifacts.close()
            raise
    return capture


@contextmanager
def operation():
    """Close only this operation's captures, after its final checks/publication."""
    if _OPERATION.get() is not None:
        yield
        return
    captures = []
    token = _OPERATION.set(captures)
    try:
        yield
    finally:
        try:
            with ExitStack() as cleanup:
                for artifacts in captures:
                    cleanup.callback(artifacts.close)
        finally:
            _OPERATION.reset(token)


def run_cli(main, *args):
    """Give CLI cancellation the same unwind path as a failed proof."""
    global _CANCEL_SIGNAL
    if _OPERATION.get() is None:
        _CANCEL_SIGNAL = signal.SIGTERM
        _CANCELLED.clear()
    previous = {}
    def cancel(signum, frame):
        global _CANCEL_SIGNAL
        _CANCEL_SIGNAL = signum
        _CANCELLED.set()
        # Allow owned runners to finish teardown after repeated Ctrl-C/TERM.
        for installed in previous:
            signal.signal(installed, signal.SIG_IGN)
        _signal_owned(signal.SIGTERM)
        if not _SPAWNING_MAIN:
            _raise_cancelled()
    if threading.current_thread() is threading.main_thread():
        for signum, default in ((signal.SIGTERM, signal.SIG_DFL),
                                (signal.SIGINT, signal.default_int_handler)):
            current = signal.getsignal(signum)
            if current in (default, signal.SIG_DFL):
                previous[signum] = current
                signal.signal(signum, cancel)
    try:
        with operation():
            return main(*args)
    finally:
        for signum, handler in previous.items():
            signal.signal(signum, handler)


# Owned subprocess groups are separate from evidence leases. Never delete a
# capture until the runner that can write it has drained and reaped its child.
_PROCESSES = set()
_PROCESS_LOCK = threading.RLock()
_CANCELLED = threading.Event()
_CANCEL_SIGNAL = signal.SIGTERM
_SPAWNING_MAIN = False
_TERM_GRACE = 30


def _signal_owned(signum):
    with _PROCESS_LOCK:
        processes = tuple(_PROCESSES)
    for process in processes:
        try:
            os.killpg(process.pid, signum)
        except (ProcessLookupError, PermissionError):
            pass


def _raise_cancelled():
    if _CANCEL_SIGNAL == signal.SIGINT:
        raise KeyboardInterrupt
    raise SystemExit(128 + _CANCEL_SIGNAL)


def _drain_owned(process):
    try:
        os.killpg(process.pid, signal.SIGTERM)
    except (ProcessLookupError, PermissionError):
        pass
    try:
        try:
            return process.communicate(timeout=_TERM_GRACE)
        except subprocess.TimeoutExpired:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except (ProcessLookupError, PermissionError):
                pass
            return process.communicate()
    finally:
        # A leader can exit while silent descendants still own output files.
        # Match bounded_capture's final group sweep before releasing evidence.
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except (ProcessLookupError, PermissionError):
            pass


def run_owned(args, *, input=None, capture_output=False, check=False, timeout=None, **kwargs):
    """The proof chain's subprocess.run interface, with graceful group teardown.

    Supports its existing cwd/env, text, stdout/stderr and timeout/check calls.
    A cancelled CLI also refuses queued thread-pool launches. Each runner's
    owning thread alone drains its pipes and reaps its process.
    """
    global _SPAWNING_MAIN
    if input is not None:
        if 'stdin' in kwargs:
            raise ValueError('stdin and input arguments may not both be used')
        kwargs['stdin'] = subprocess.PIPE
    if capture_output:
        if 'stdout' in kwargs or 'stderr' in kwargs:
            raise ValueError('stdout/stderr and capture_output may not both be used')
        kwargs.update(stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if _CANCELLED.is_set():
        _raise_cancelled()
    on_main = threading.current_thread() is threading.main_thread()
    process = None
    try:
        if on_main:
            _SPAWNING_MAIN = True
        try:
            process = subprocess.Popen(args, start_new_session=True, **kwargs)
            with _PROCESS_LOCK:
                _PROCESSES.add(process)
        finally:
            if on_main:
                _SPAWNING_MAIN = False
        if _CANCELLED.is_set():
            _raise_cancelled()
        stdout, stderr = process.communicate(input, timeout=timeout)
        if process.returncode:
            _drain_owned(process)
        if _CANCELLED.is_set():
            _raise_cancelled()
    except BaseException as error:
        if process is not None:
            stdout, stderr = _drain_owned(process)
            error.output, error.stderr = stdout, stderr
        raise
    finally:
        if process is not None:
            with _PROCESS_LOCK:
                _PROCESSES.discard(process)
            if process.stdout is not None:
                process.stdout.close()
            if process.stderr is not None:
                process.stderr.close()
    result = subprocess.CompletedProcess(args, process.returncode, stdout, stderr)
    if check:
        result.check_returncode()
    return result
