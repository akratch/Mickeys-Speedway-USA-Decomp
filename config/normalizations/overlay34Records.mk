$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34ResetStorage.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym overlay34RemoveRecord=overlay34RemoveRecordReloc \
		--redefine-sym func_8002B768=overlay34FreeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94

# Exact C emits the source-facing symbol directly. Bind its local remove call
# through the overlay's zero-field proxy so the shipped runtime relocation
# supplies the authenticated +0x2C8 identity, then discard only IDO's trailing
# section alignment outside the function's 0x134-byte owned range.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34UpdateRecords.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym overlay34RemoveRecord=overlay34RemoveRecordReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134

$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SortAndDraw.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_80024938=func_80024938_o034Reloc \
		--redefine-sym func_800084C4=func_800084C4_o034Reloc \
		--redefine-sym overlay34InterpolateColor=overlay34InterpolateColorReloc \
		$@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F8
