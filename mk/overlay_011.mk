# Overlay 11 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11EnableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# The menu background draw is instruction-exact. Its .rodata (two 0.05f
# pool entries and the six-entry mode switch table) is the retained overlay
# rodata at +0x20: assert the compiler's copy by digest and anchor its three
# references there. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0000150_1868998.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0000150_1868998.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:3cf0f06d2f930b6c90429208150d97e1f581e7c06d733df19d664be285427112 0x20 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11DisableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
# These one-to-one proxies describe the thirteen runtime-authenticated call
# roles required by a future C promotion. The current NON_MATCHING fallback
# does not emit the friendly names, so its ordinary object leaves these
# mappings inert until the candidate body becomes canonical.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_011_F0001398_1869BE0=overlay11UpdateMenu \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym overlay66Select=overlay11Overlay66SelectReloc \
		--redefine-sym func_800290AC=overlay11ResidentModeReloc \
		--redefine-sym func_800291D8=overlay11Func800291D8Reloc \
		--redefine-sym func_800006BC=overlay11Func800006BCReloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11Func2BF4Reloc \
		--redefine-sym func_80005820=overlay11Func80005820Reloc \
		--redefine-sym func_80028374=overlay11Func80028374Reloc \
		--redefine-sym func_80028528=overlay11Func80028528Reloc \
		--redefine-sym func_8003A754=overlay11Func8003A754Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4B4
# Overlay-local data addends are encoded in retail, while its runtime calls
# all use the extracted range's offset-zero carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:D_INPUT 0x4:6:D_INPUT 0x8:5:D_0 0x14:6:D_0 \
		0x5C:5:D_INPUT 0x60:6:D_INPUT \
		0x64:5:D_0_reload_success 0x70:6:D_0_reload_success \
		0x7C:5:D_INPUT 0x80:6:D_INPUT \
		0x84:5:D_0_reload_failure 0x8C:6:D_0_reload_failure \
		0xF0:5:D_menuBase 0xF8:6:D_menuBase \
		0x120:5:D_INPUT 0x128:6:D_INPUT \
		0x134:5:D_menuInputBase 0x138:6:D_menuInputBase \
		0x198:5:D_menuCounterBase 0x1A0:6:D_menuCounterBase \
		0x1A4:5:D_menuInputBase 0x1A8:6:D_menuInputBase \
		0x214:5:D_menuCounterBase 0x218:6:D_menuCounterBase && \
	$(OBJCOPY) --redefine-sym \
		func_80000F94=func_overlay_011_F0000000_1868848 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x168:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x170:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x178:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x188:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x190:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x1E8:func_80028528:func_overlay_011_F0000000_1868848 \
		0x20C:func_80028374:func_overlay_011_F0000000_1868848
# The compiler emits the exact five-entry switch table already present at
# overlay-local +0x40. Rebind the text pair there, discard only the duplicate
# private table, and preserve the retail offset-zero runtime call carriers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=func_overlay_011_F0000000_1868848 \
		--add-symbol gOverlay11FiveOptionSwitchTableReloc=0x40,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x164:.rodata:gOverlay11FiveOptionSwitchTableReloc \
		0x16C:.rodata:gOverlay11FiveOptionSwitchTableReloc \
		0x178:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x180:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x188:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x198:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x1A0:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x218:func_80005820:func_overlay_011_F0000000_1868848 \
		0x220:func_8002675C:func_overlay_011_F0000000_1868848 \
		0x240:func_80028374:func_overlay_011_F0000000_1868848 \
		0x2B0:func_80028374:func_overlay_011_F0000000_1868848 \
		0x320:func_80028374:func_overlay_011_F0000000_1868848 \
		0x3A8:func_80028374:func_overlay_011_F0000000_1868848 && \
	$(OBJCOPY) --remove-section=.rodata $@
# The six-option menu update is instruction-exact. Its ten resident callees
# go through the generated surface entries, and its switch table is the
# retained overlay table at rodata +0x54: bind the two table references to
# that owner and drop the compiler's private copy by digest. No instruction
# changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_011_F0001E4C_186A694.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o011Reloc \
		--redefine-sym amTuneSetFadeScaled=amTuneSetFadeScaled_o011Reloc \
		--redefine-sym func_80005820=func_80005820_o011Reloc \
		--redefine-sym func_80028F54=func_80028F54_o011Reloc \
		--redefine-sym func_800290AC=func_800290AC_o011Reloc \
		--redefine-sym func_800291D8=func_800291D8_o011Reloc \
		--redefine-sym joyGetPressed=joyGetPressed_o011Reloc \
		--redefine-sym levelGetNumber=levelGetNumber_o011Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o011Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o011Reloc \
		--add-symbol gOverlay11OptionSwitchTableReloc=0x54,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F0001E4C_186A694.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:48290777f7df3b6d1161aa730c2fc4fb1c29eb0ee77b5e395dd5fbc4dc145418 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x49C
# The sibling option-menu update is instruction-exact, written as the copy of
# func_overlay_011_F0001E4C_186A694 above. Its resident callees go through
# the generated surface entries, and its switch table is the retained
# overlay table at rodata +0x68: bind the two table references to that owner
# and drop the compiler's private copy by digest. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_011_F00022E8_186AB30.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o011Reloc \
		--redefine-sym amTuneSetFadeScaled=amTuneSetFadeScaled_o011Reloc \
		--redefine-sym func_80005820=func_80005820_o011Reloc \
		--redefine-sym func_80028F54=func_80028F54_o011Reloc \
		--redefine-sym func_800290AC=func_800290AC_o011Reloc \
		--redefine-sym func_800291D8=func_800291D8_o011Reloc \
		--redefine-sym joyGetPressed=joyGetPressed_o011Reloc \
		--redefine-sym levelGetNumber=levelGetNumber_o011Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o011Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o011Reloc \
		--add-symbol gOverlay11ModeOptionSwitchTableReloc=0x68,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_011_F00022E8_186AB30.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:dc72499309bd6a0f4e3c714903a5310fd3406829d2857b1c3abffd88023f2868 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x42C
# Overlay-local data addends are encoded in retail, while its runtime calls
# use the extracted range's offset-zero carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:D_INPUT 0x4:6:D_INPUT 0x8:5:D_0 0x14:6:D_0 \
		0x5C:5:D_INPUT 0x60:6:D_INPUT \
		0x64:5:D_0_reload_success 0x70:6:D_0_reload_success \
		0x7C:5:D_INPUT 0x80:6:D_INPUT \
		0x84:5:D_0_reload_failure 0x8C:6:D_0_reload_failure \
		0xF0:5:D_menuBase 0xF8:6:D_menuBase \
		0x120:5:D_INPUT 0x128:6:D_INPUT \
		0x134:5:D_menuInputBase 0x138:6:D_menuInputBase \
		0x198:5:D_menuCounterBase 0x1A0:6:D_menuCounterBase \
		0x1A4:5:D_menuInputBase 0x1A8:6:D_menuInputBase \
		0x1F4:5:D_lastMode 0x1F8:6:D_lastMode \
		0x218:5:D_menuCounterBase 0x21C:6:D_menuCounterBase && \
	$(OBJCOPY) --redefine-sym \
		func_80000F94=func_overlay_011_F0000000_1868848 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x168:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x170:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x178:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x188:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x190:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x210:func_80028374:func_overlay_011_F0000000_1868848 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x234
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11CreateHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
# The compiler emits the exact six-entry switch table already present in the
# overlay's extracted data/rodata asset. Rebind the text pair to its proved
# runtime-local `+8` addend, then discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_800290AC=overlay11ResidentModeReloc \
		--redefine-sym func_800005CC=overlay11ResidentFloatReloc \
		--redefine-sym overlay66Select=overlay11Overlay66SelectReloc \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym func_8004B0A4=overlay11DrawModeReloc \
		--redefine-sym func_8004B0B8=overlay11DrawColorReloc $@ && \
	$(OBJCOPY) --add-symbol gOverlay11SwitchTableReloc=0x8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xA8:.rodata:gOverlay11SwitchTableReloc \
		0xB0:.rodata:gOverlay11SwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym sprintf=overlay11FormatReloc \
		--redefine-sym D_800D31BC=gOverlay11ResidentFlagsReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
# The compiler emits the exact six-entry switch table already present in the
# overlay's extracted data/rodata asset. Rebind the text pair to its proved
# runtime-local +0x7C addend, then discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay11ReleaseSwitchTableReloc=0x7C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x30:.rodata:gOverlay11ReleaseSwitchTableReloc \
		0x38:.rodata:gOverlay11ReleaseSwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
