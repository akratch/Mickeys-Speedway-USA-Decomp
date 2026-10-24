# Overlay 52: one complete source TU with runtime section-relative linking.
# Preserve the original compiler object and all475 source relocation records;
# ordinary module linking resolves real local storage and canonical ORT imports.
MODULE_LINKED_OVERLAYS += 052
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay_052.c.o: CFLAGS += -Wab,-r4300_mul
# The common compiler-metadata step restores static procedures, BSS and DATA.
