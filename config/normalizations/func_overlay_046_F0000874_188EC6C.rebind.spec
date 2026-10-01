# Bind the three step-factor literal references to the retained overlay pool.
# The compiler places the factors at +0x28..+0x30 of its private section; the
# pool base symbol carries the difference to the retained +0x4C..+0x54. Only
# relocation symbol indices change; instructions stay untouched.
0xB4:.rodata:gOverlay46ParticleStepPoolReloc
0xB8:.rodata:gOverlay46ParticleStepPoolReloc
0x1D0:.rodata:gOverlay46ParticleStepPoolReloc
0x1D4:.rodata:gOverlay46ParticleStepPoolReloc
0x434:.rodata:gOverlay46ParticleStepPoolReloc
0x438:.rodata:gOverlay46ParticleStepPoolReloc
