# Overlay 58 rules, split out of mk/overlays.mk on 2026-10-07 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x5A20 bytes, trailing pad included, so no trim, and resident and
# cross-overlay calls are renamed by the generated surface rebinds. Kept
# until phase 3: the TU's .rodata (the state update's seven-entry switch
# table and 0.02f literal, then the mode update's thirteen-entry table) is
# still owned by the raw data blob at data_rodata +0x3D4, so the section is
# asserted by digest and its three %hi/%lo pairs bound to a pool symbol at the shipped
# rodata-relative 0x104; the tables' own text relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay58RodataReloc=0x104,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x654:.rodata:gOverlay58RodataReloc 0x65C:.rodata:gOverlay58RodataReloc 0xFF4:.rodata:gOverlay58RodataReloc 0xFF8:.rodata:gOverlay58RodataReloc \
		0x149C:.rodata:gOverlay58RodataReloc 0x14A4:.rodata:gOverlay58RodataReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:3dbf1ba8a2f53fbf224e3e1c3b43aca87ad4e03ba1a51538cbd7e818f0d158be && \
	$(OBJCOPY) --remove-section=.rel.rodata $@

# Complete ordinary mutable transition input; compiler output is unmodified.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58TransitionState.c.o: private PLAIN_DATA_PROOF = 1
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58TransitionState.c.o: private PLAIN_DATA_RUNTIME_ROLE = data

# ADR0026: one genuine mixed rank/glyph aggregate targets actual readonly
# named bases; the complete provider input declares runtime RO origin.
MODULE_LINKED_DATA_SOURCES += overlays/o058/overlay58RankTextLayout
O58_RANK_LAYOUT := $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58RankTextLayout.c.o
O58_RANK_LABELS := $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58TextLabels.c.o
$(O58_RANK_LAYOUT): $(O58_RANK_LABELS)
$(O58_RANK_LAYOUT): private LOCAL_RO_ORIGIN_OBJECT = $(O58_RANK_LABELS)
$(O58_RANK_LAYOUT): private LOCAL_DATA_AGGREGATE = 1

# The selection/path-node map ends at its genuine sentinel-backed object.
# Preserve the compiler object; discard only its derived zero end alignment.
O58_PATH_MAP := $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58PathMap.c.o
$(O58_PATH_MAP): $(TOOLS_DIR)/trim_elf_section.py
$(O58_PATH_MAP): private POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x54 --compiler-end-padding
$(O58_PATH_MAP): private PLAIN_DATA_PROOF = 1
$(O58_PATH_MAP): private PLAIN_DATA_RUNTIME_ROLE = data

# Six actual tie-placement awards; keep the original compiler object and
# remove only the independently derived zero end alignment.
O58_RANK_AWARDS := $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58RankAwards.c.o
$(O58_RANK_AWARDS): $(TOOLS_DIR)/trim_elf_section.py
$(O58_RANK_AWARDS): private POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x18 --compiler-end-padding
$(O58_RANK_AWARDS): private PLAIN_DATA_PROOF = 1
$(O58_RANK_AWARDS): private PLAIN_DATA_RUNTIME_ROLE = data
