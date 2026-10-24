# Overlay 57 rules, split out of mk/overlays.mk on 2026-10-02 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x6AB0 bytes, trailing pad included, so no trim, and resident calls are
# renamed by the generated surface rebinds. -woff 835: several resident
# callees are deliberately unprototyped (overlay_057.h). Two steps stay, both
# for a mechanical reason:
#
# - .rodata: retain the initializer's fifteen-entry switch table, two blend
#   literals and the dispatch's twenty-one-entry table at data +0x5AC..+0x644.
#   Source-proved compiler end alignment removes only the final eight bytes.
#   The original section supplies the loader's rodata-relative 0x6C anchor;
#   the runtime table owns the thirty-six R32 records on its text targets.
# - .bss: the TU defines the module's BSS block (overlay_057.c), so every
#   access is a section-relative record whose addend is already the offset the
#   shipped word carries. Overlay 57's runtime table owns those 73 HI16/LO16
#   records, so the static link must not adjust them: they are removed,
#   asserted one by one. No instruction word is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x98 --compiler-end-padding && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:37586b7bbc15e9a7597ad11751411fbfea75cdc79761005a9446ee88296cb78b 0x6C keep && \
	$(OBJCOPY) --remove-section=.rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x1AE8:5:.bss 0x1AEC:6:.bss 0x1E84:5:.bss 0x1E90:6:.bss \
		0x254C:5:.bss 0x2550:6:.bss 0x2F54:5:.bss 0x2F58:6:.bss \
		0x3000:5:.bss 0x3008:6:.bss 0x3054:5:.bss 0x3060:6:.bss \
		0x3204:5:.bss 0x3208:6:.bss 0x3FDC:5:.bss 0x3FE8:6:.bss \
		0x3FFC:5:.bss 0x4008:6:.bss 0x4000:5:.bss 0x400C:6:.bss \
		0x4070:5:.bss 0x4078:6:.bss 0x4228:5:.bss 0x422C:6:.bss \
		0x43D0:5:.bss 0x43DC:6:.bss 0x43EC:5:.bss 0x43F4:6:.bss \
		0x43E8:5:.bss 0x43F0:6:.bss 0x4468:5:.bss 0x4478:6:.bss \
		0x4B48:5:.bss 0x4B4C:6:.bss 0x4C18:5:.bss 0x4C24:6:.bss \
		0x4C20:5:.bss 0x4C28:6:.bss 0x4C54:5:.bss 0x4C5C:6:.bss \
		0x4C74:5:.bss 0x4C94:6:.bss 0x4C8C:6:.bss 0x4C78:5:.bss \
		0x4C7C:6:.bss 0x4CCC:5:.bss 0x4CD0:6:.bss 0x4CD4:5:.bss \
		0x4CDC:6:.bss 0x4CD8:5:.bss 0x4CE0:6:.bss 0x4CF0:5:.bss \
		0x4CF8:6:.bss 0x4CFC:5:.bss 0x4D04:6:.bss 0x4D0C:5:.bss \
		0x4D1C:6:.bss 0x4D30:5:.bss 0x4D38:6:.bss 0x4D24:5:.bss \
		0x4D3C:6:.bss 0x4D44:5:.bss 0x4D50:6:.bss 0x4D70:5:.bss \
		0x4D74:6:.bss 0x4D9C:5:.bss 0x4DA8:6:.bss 0x4DB4:5:.bss \
		0x4DBC:6:.bss 0x4DB8:5:.bss 0x4DC0:6:.bss 0x6478:5:.bss \
		0x647C:6:.bss

# Independently bounded mutable state objects. The unknown intervening word
# remains retained data; only source-proved compiler end alignment is removed.
O57_SELECTION_FLAG := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SelectionFlag.c.o
O57_TRANSITION_STATE := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57TransitionState.c.o
$(O57_SELECTION_FLAG) $(O57_TRANSITION_STATE): $(TOOLS_DIR)/trim_elf_section.py
$(O57_SELECTION_FLAG): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x4 --compiler-end-padding
$(O57_TRANSITION_STATE): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x18 --compiler-end-padding
$(O57_SELECTION_FLAG) $(O57_TRANSITION_STATE): private PLAIN_DATA_PROOF = 1
$(O57_SELECTION_FLAG) $(O57_TRANSITION_STATE): private PLAIN_DATA_RUNTIME_ROLE = data

# Complete resource/ID family; actual sized objects end at 0xFC. Only its
# independently proved four-byte compiler end alignment is excluded.
O57_PATH_RESOURCES := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57PathResources.c.o
$(O57_PATH_RESOURCES): $(TOOLS_DIR)/trim_elf_section.py
$(O57_PATH_RESOURCES): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0xFC --compiler-end-padding
$(O57_PATH_RESOURCES): private PLAIN_DATA_PROOF = 1
$(O57_PATH_RESOURCES): private PLAIN_DATA_RUNTIME_ROLE = data

# Genuine sized selector/colour inputs: retain their complete object ranges
# and remove only independently derived compiler end alignment.
O57_DETAIL_COLOURS := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57DetailColours.c.o
O57_UNLOCK_LABELS := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UnlockLabels.c.o
O57_MODE_TABLES := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ModeTables.c.o
$(O57_DETAIL_COLOURS) $(O57_UNLOCK_LABELS) $(O57_MODE_TABLES): $(TOOLS_DIR)/trim_elf_section.py
$(O57_DETAIL_COLOURS): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x18 --compiler-end-padding
$(O57_UNLOCK_LABELS): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x14 --compiler-end-padding
$(O57_MODE_TABLES): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x24 --compiler-end-padding
$(O57_DETAIL_COLOURS) $(O57_UNLOCK_LABELS) $(O57_MODE_TABLES): private PLAIN_DATA_PROOF = 1
$(O57_DETAIL_COLOURS) $(O57_UNLOCK_LABELS) $(O57_MODE_TABLES): private PLAIN_DATA_RUNTIME_ROLE = data

# Complete literal family: retain original raw48/sizes0 and prove end44 from
# genuine source/CPP arrays. Its loader RO base is the actual readonly input.
O57_RESULT_LABELS := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ResultLabels.c.o
$(O57_RESULT_LABELS): $(TOOLS_DIR)/trim_elf_section.py $(TOOLS_DIR)/proof_provenance.py $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57MenuLabels.c.o
$(O57_RESULT_LABELS): private POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x2C --literal-array-end src/overlays/o057/overlay57ResultLabels.c
$(O57_RESULT_LABELS): private PLAIN_DATA_PROOF = 1
$(O57_RESULT_LABELS): private PLAIN_DATA_RUNTIME_ROLE = ro-member
$(O57_RESULT_LABELS): private PLAIN_DATA_RO_ORIGIN_OBJECT = build/src/overlays/o057/overlay57MenuLabels.c.o
$(O57_RESULT_LABELS): private PLAIN_DATA_RO_ORIGIN_SECTION = .rodata

# Genuine ten-entry character order. Only source-derived compiler end
# alignment is removed; the original32-byte compiler object stays in proof.
O57_CHARACTER_ORDER := $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57CharacterOrder.c.o
$(O57_CHARACTER_ORDER): $(TOOLS_DIR)/trim_elf_section.py
$(O57_CHARACTER_ORDER): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x14 --compiler-end-padding
$(O57_CHARACTER_ORDER): private PLAIN_DATA_PROOF = 1
$(O57_CHARACTER_ORDER): private PLAIN_DATA_RUNTIME_ROLE = data
