# Overlay 57 rules, split out of mk/overlays.mk on 2026-10-02 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x6AB0 bytes, trailing pad included, so no trim, and resident calls are
# renamed by the generated surface rebinds. -woff 835: several resident
# callees are deliberately unprototyped (overlay_057.h). Two steps stay, both
# for a mechanical reason:
#
# - .rodata: the initializer's and the dispatch's switch tables and the grid
#   draw's two blend literals between them are still owned by the raw data
#   blob at data_rodata +0x5AC (phase 3), so the section is asserted by digest
#   and its four %hi/%lo pairs bound to a pool symbol at the shipped rodata-relative 0x6C;
#   the tables' own text relocations go with it.
# - .bss: the TU defines the module's BSS block (overlay_057.c), so every
#   access is a section-relative record whose addend is already the offset the
#   shipped word carries. Overlay 57's runtime table owns those 73 HI16/LO16
#   records, so the static link must not adjust them: they are removed,
#   asserted one by one. No instruction word is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay_057.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay57RodataReloc=0x6C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x578:.rodata:gOverlay57RodataReloc 0x580:.rodata:gOverlay57RodataReloc 0x10AC:.rodata:gOverlay57RodataReloc 0x10C0:.rodata:gOverlay57RodataReloc \
		0x1128:.rodata:gOverlay57RodataReloc 0x1154:.rodata:gOverlay57RodataReloc 0x1B18:.rodata:gOverlay57RodataReloc 0x1B20:.rodata:gOverlay57RodataReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:6c9d41212841e67ae005670bb37e90e37a8bca7f188c3980fff0c999c4e69a52 && \
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
