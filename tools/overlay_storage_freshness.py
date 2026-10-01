"""Final freshness checks for explicit typed-overlay storage captures."""


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
