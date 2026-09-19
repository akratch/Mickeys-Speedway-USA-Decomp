O22_REMOVE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22RemoveObject.c.o
O22_RESOLVE_PLANE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22ResolvePlane.c.o
O22_INITIALIZE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22InitializeObject.c.o
O22_UPDATE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/func_overlay_022_F00002B0_18783B8.c.o

# Phase-B reconstruction plateau. NON_MATCHING builds keep the best compiler
# candidate; ordinary builds retain the retail body and exact ownership range.
$(O22_UPDATE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_UPDATE_OBJECT_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7CC

# The compiler reproduces the retail body; the flag is load-bearing. The
# self-rename is a historical no-op metadata marker and changes no instruction.
$(O22_INITIALIZE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_INITIALIZE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000000_1878108=func_overlay_022_F0000000_1878108 $@


# The compiler reproduces the retail body; retain the exact owned text
# extent. The self-rename is a historical no-op metadata marker.
$(O22_REMOVE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000D30_1878E38=func_overlay_022_F0000D30_1878E38 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x16C

# The compiler reproduces the retail body. Overlay 22's runtime relocation
# binds the resident sqrtf call through the module base entry; restore that
# identity, then retain the exact owned text extent. No instruction changes.
$(O22_RESOLVE_PLANE_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_RESOLVE_PLANE_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=func_overlay_022_F0000000_1878108 $@ && \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000A7C_1878B84=func_overlay_022_F0000A7C_1878B84 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
