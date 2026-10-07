"""Canonical overlay call spelling and dependency inputs.

These helpers select proof inputs; they never grant relocation identity.
"""

from pathlib import Path
import re

import reloc_identity as ri


def callee_build_dependencies(root, source_path, source_text):
    """Conservative canonical dependency set for an identity-only read proof.

    This does not build or create a compiler receipt. Literal header lookup
    follows the canonical C rule (including asm-processor's source directory);
    unsupported includes fail closed. Build policy and compiler/metadata tools
    are timestamp prerequisites even where make's object rule omits them.
    """
    import permute_batch as batch
    import subprocess
    import time
    root = Path(root)
    if root.resolve() != batch.ROOT.resolve():
        return None
    try:
        deadline = time.monotonic() + 5
        recipe = batch.build_recipe_for(source_path, deadline=deadline)
        if not recipe.from_dry_run or not recipe.compiler_args:
            return None
        # asm-processor appends the original source directory to the real
        # compiler's -I list because its temporary C lives elsewhere.
        includes = batch.source_dependencies(
            source_path, recipe.compiler_args + ("-I", str(source_path.parent)), deadline)
    except (OSError, RuntimeError, ValueError, subprocess.SubprocessError):
        return None
    if any(name.startswith("missing:") for name in includes):
        return None
    dependencies = set(path for path in (root / "include").rglob("*")
                       if path.is_file())
    dependencies.update(path for path in (root / "mk").rglob("*.mk") if path.is_file())
    # Rebind/filter specs are build inputs too; their suffix is not uniformly
    # JSON and editing them need not alter the expanded command string.
    dependencies.update(path for path in (root / "config/normalizations").rglob("*")
                        if path.is_file())
    dependencies.update(path for path in (root / "tools/ido").rglob("*")
                        if path.is_file())
    dependencies.update(path for path in (root / "tools/asm-processor").glob("*.py")
                        if path.is_file())
    for relative in (
            "Makefile", "build/.splat-stamp", "tools/binutils/mips64-elf-as",
            "tools/binutils/mips64-elf-objcopy", "tools/normalize_elf_instructions.py",
            "tools/filter_elf_relocations.py", "tools/trim_elf_section.py",
            "tools/externalize_elf_section.py", "tools/rebind_elf_relocations.py",
            "tools/set_elf_flags.py", "tools/render_overlay_aliases.py"):
        path = root / relative
        if path.is_file():
            dependencies.add(path)
    dependencies.update(root / name for name in includes)
    return dependencies, (recipe, includes)


def overlay_call_definition_name(root, name, generated_name_re):
    """Select a spelling, not an identity, from one direct canonical alias.

    No numeric assignment, shared VMA, alias chain or runtime-site correlation
    establishes this relation. The caller must independently prove the selected
    ordinary C definition, object boundary, exact ownership and linked bytes.
    """
    path = Path(root) / "overlay_undefined_syms.us.txt"
    if not path.is_file():
        return (name, name, None, None) if generated_name_re.fullmatch(name) else None
    contents = path.read_text()
    pairs = ri.parse_linker_aliases(contents)
    related = [pair for pair in pairs if name in pair]
    if not related:
        return (name, name, None, None) if generated_name_re.fullmatch(name) else None
    if len(related) != 1:
        raise ValueError("ambiguous canonical overlay call alias")
    generated, friendly = related[0]
    if not generated_name_re.fullmatch(generated) or generated_name_re.fullmatch(friendly):
        return None
    if any(pair != related[0] and (generated in pair or friendly in pair)
           for pair in pairs):
        raise ValueError("canonical overlay call alias is not unique and direct")
    # A second numeric/expression assignment must not hide behind the narrow
    # equality parser, nor can a duplicate equality establish unique authority.
    assignments = re.findall(r"^\s*([A-Za-z_.$][A-Za-z0-9_.$]*)\s*=", contents, re.M)
    if assignments.count(generated) != 1 or assignments.count(friendly):
        raise ValueError("conflicting canonical overlay call alias assignments")
    return generated, friendly, path, contents
