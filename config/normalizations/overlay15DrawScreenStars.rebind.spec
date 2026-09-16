# Bind the compiler's private pool reference (one float constant at +0x0, the
# fade scale 255.0f / 292.0f of overlay15DrawScreenStars) to the retained
# overlay data word at data_rodata +0x40, which the shipped LOCAL %hi/%lo pair
# encodes as immediates 0x0000/0x0000 with the loader supplying the base.  Only
# relocation symbol indices change; instructions and addends stay untouched.
0x560:.rodata:gOverlay15FadePoolReloc
0x564:.rodata:gOverlay15FadePoolReloc
