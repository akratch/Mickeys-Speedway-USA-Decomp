"""Final freshness checks for explicit typed-overlay storage captures."""

import hashlib
from pathlib import Path


_LOADED_IMPLEMENTATION_SHA256 = hashlib.sha256(
    Path(__file__).read_bytes()).hexdigest()


def require_loaded_implementation(error_type):
    """Reject an extracted helper changed after this module was imported."""
    if hashlib.sha256(Path(__file__).read_bytes()).hexdigest() != _LOADED_IMPLEMENTATION_SHA256:
        raise error_type("loaded storage-freshness implementation changed on disk")


def checked_tool_identity(batch, error_type):
    try:
        return batch.checked_tool_identity()
    except RuntimeError as error:
        raise error_type("loaded proof tool changed during explicit storage proof") from error


def source_dependency_snapshot(batch, source, modes):
    return {
        row["mode"]: batch.source_dependencies(
            source, tuple(row["compiler_arguments"]))
        for row in modes
    }


def require_dependency_snapshot(actual, expected, error_type):
    if actual != expected:
        raise error_type("explicit storage source dependencies changed before resolution completed")
