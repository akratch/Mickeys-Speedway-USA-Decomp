#!/usr/bin/env python3
"""Run one command with complete logs and a compact terminal receipt."""

from __future__ import annotations

import argparse
from pathlib import Path
import shlex
import subprocess
import sys
import time

from proof_artifacts import run_owned


class LoggedRunError(ValueError):
    pass


def resolve_log(repo: Path, value: Path) -> Path:
    repo = repo.resolve()
    build = (repo / "build").resolve()
    path = value if value.is_absolute() else repo / value
    path = path.resolve()
    try:
        path.relative_to(build)
    except ValueError as exc:
        raise LoggedRunError("--log must remain under the repository build/") from exc
    return path


def tail_lines(path: Path, count: int) -> list[str]:
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return []
    return lines[-count:]


def run_logged(
    repo: Path,
    log_path: Path,
    label: str,
    command: list[str],
    *,
    tail_count: int = 30,
) -> int:
    if not command:
        raise LoggedRunError("a command is required after --")
    if tail_count <= 0:
        raise LoggedRunError("--tail-lines must be positive")
    repo = repo.resolve()
    log_path = resolve_log(repo, log_path)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    with log_path.open("wb") as log:
        rendered = shlex.join(command)
        log.write(f"command: {rendered}\n".encode())
        log.flush()
        try:
            result = run_owned(
                command, cwd=repo, stdout=log, stderr=subprocess.STDOUT,
            )
        except OSError as exc:
            log.write(f"launch failed: {exc}\n".encode())
            print(f"{label} FAIL 0.0s log={log_path}", file=sys.stderr)
            return 127
        return_code = result.returncode
    elapsed = time.monotonic() - started
    status = "PASS" if return_code == 0 else "FAIL"
    stream = sys.stdout if return_code == 0 else sys.stderr
    print(f"{label} {status} {elapsed:.1f}s log={log_path}", file=stream)
    if return_code != 0:
        for line in tail_lines(log_path, tail_count):
            print(line, file=sys.stderr)
    return return_code


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(description=__doc__)
    result.add_argument("--repo", type=Path, default=Path.cwd())
    result.add_argument("--log", type=Path, required=True)
    result.add_argument("--label", required=True)
    result.add_argument("--tail-lines", type=int, default=30)
    result.add_argument("command", nargs=argparse.REMAINDER)
    return result


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    command = args.command[1:] if args.command[:1] == ["--"] else args.command
    try:
        return run_logged(
            args.repo,
            args.log,
            args.label,
            command,
            tail_count=args.tail_lines,
        )
    except LoggedRunError as exc:
        print(f"run-logged: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    from proof_artifacts import run_cli
    raise SystemExit(run_cli(main))
