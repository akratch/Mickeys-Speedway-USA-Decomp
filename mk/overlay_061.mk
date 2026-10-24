# Overlay 61 rules, split out of mk/overlays.mk on 2026-10-07 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x1A90 bytes, trailing pad included, so no trim, and resident calls are
# renamed by the generated surface rebinds. -woff 835: four resident callees
# are deliberately unprototyped (overlay_061.h). Kept until phase 3: the
# TU's .rodata (the pak menu's nine-entry switch table) is still owned by the
# raw data blob, so the section is asserted by digest and its %hi/%lo pair
# bound to a pool symbol at the shipped rodata-relative 0x188; the table's own text
# relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay61RodataReloc=0x188,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xC9C:.rodata:gOverlay61RodataReloc 0xCA4:.rodata:gOverlay61RodataReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:7d32dd8918ee69dd189577037beffda3b905ceda99ea1e8fadd03d362f556145 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@

O61_INITIAL_STATE := $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitialState.c.o
$(O61_INITIAL_STATE): $(TOOLS_DIR)/trim_elf_section.py
$(O61_INITIAL_STATE): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0xB4 --compiler-end-padding
$(O61_INITIAL_STATE): private PLAIN_DATA_PROOF = 1
$(O61_INITIAL_STATE): private PLAIN_DATA_RUNTIME_ROLE = data

# Complete ordinary mutable string inputs: no end trimming or const coercion.
# Actual first character-name input supplies the runtime readonly origin;
# every following member holds that same current source/map identity.
O61_CHARACTER_NAMES := $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61CharacterNames.c.o
O61_STRING_MEMBERS := $(addprefix $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/,$(addsuffix .c.o,overlay61GhostFormats overlay61DirectoryFormats overlay61MenuLabels overlay61FileNames))
$(O61_CHARACTER_NAMES) $(O61_STRING_MEMBERS): private PLAIN_DATA_PROOF = 1
$(O61_CHARACTER_NAMES): private PLAIN_DATA_RUNTIME_ROLE = ro-origin
$(O61_STRING_MEMBERS): private PLAIN_DATA_RUNTIME_ROLE = ro-member
$(O61_STRING_MEMBERS): private PLAIN_DATA_RO_ORIGIN_OBJECT = build/src/overlays/o061/overlay61CharacterNames.c.o
