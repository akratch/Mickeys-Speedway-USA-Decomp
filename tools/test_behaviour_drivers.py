"""The anim.c behaviour drivers against the tracked source (host cc, no build).

These compile src/main/anim.c for the host, which is tracked C, so they need
no ROM. A test that cannot find the line it mutates skips rather than fails:
the source moves on, and the mutation then needs rewriting, not the driver.
"""

import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import behaviour_check as bc  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
ANIM = ROOT / "src" / "main" / "anim.c"
INTERPRETER = ROOT / "tools" / "behaviour_drivers" / "anim_interpreter.c"
COLLISION = ROOT / "tools" / "behaviour_drivers" / "anim_collision.c"


@unittest.skipUnless(shutil.which("cc") and ANIM.exists(), "no host C compiler or no anim.c")
class AnimDriverTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = Path(tempfile.mkdtemp(prefix="test_behaviour_drivers_"))
        cls.saved_cache = bc.CACHE_DIR
        bc.CACHE_DIR = cls.tmp / "cache"  # the reference is built once per driver

    @classmethod
    def tearDownClass(cls):
        bc.CACHE_DIR = cls.saved_cache
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def mutant(self, name, old, new):
        text = ANIM.read_text()
        if old not in text:
            self.skipTest(f"anim.c no longer contains {old.strip()!r}")
        p = self.tmp / name
        p.write_text(text.replace(old, new, 1))
        return p

    def verdict(self, candidate, driver):
        return bc.same_behaviour(candidate, ANIM, driver)[0]

    def test_interpreter_canonical_is_itself(self):
        self.assertEqual(self.verdict(ANIM, INTERPRETER), 0)

    def test_interpreter_rejects_the_s8_carrier(self):
        # 0x60's byte as s8 passes a negative frame for 0x80..0xFF, which
        # func_8005AD64 clamps to 0: a different frame.
        cand = self.mutant("s8.c", "    s16 signedValue;\n", "    s8 signedValue;\n")
        self.assertEqual(self.verdict(cand, INTERPRETER), 1)

    def test_interpreter_rejects_a_dropped_mask(self):
        cand = self.mutant("mask.c", "                    word2 = cursor->command & 0xFF;\n"
                                     "                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);\n"
                                     "                    func_8004EED0(word2);",
                           "                    word2 = cursor->command;\n"
                           "                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 4);\n"
                           "                    func_8004EED0(word2);")
        self.assertEqual(self.verdict(cand, INTERPRETER), 1)

    def test_interpreter_accepts_a_big_endian_equivalent_byte_load(self):
        # The byte at +5 is the low byte of the halfword at +4 on the N64.
        cand = self.mutant("byte.c", "                    type = *((u8 *) cursor + 5);\n"
                                     "                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);",
                           "                    type = *((u16 *) ((u8 *) cursor + 4)) & 0xFF;\n"
                           "                    cursor = (AnimStreamEntry *) ((u8 *) cursor + 6);")
        self.assertEqual(self.verdict(cand, INTERPRETER), 0)

    def test_collision_canonical_is_itself(self):
        self.assertEqual(self.verdict(ANIM, COLLISION), 0)

    def test_collision_rejects_a_changed_constant(self):
        cand = self.mutant("c83.c", "radiusSq + 83.0f", "radiusSq + 82.0f")
        self.assertEqual(self.verdict(cand, COLLISION), 1)


if __name__ == "__main__":
    unittest.main()
