# Overlay 84 rules. Included from mk/overlays.mk.
# The module is one translation unit under the overlay flag set; its .text is
# the module's 0x13C0 bytes, trailing pad included, so it needs no trim. Its
# three unexported functions are static (JUMP records).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay_084.c.o: CFLAGS += -Wab,-r4300_mul
OVERLAY_STATIC_FUNCTION_OBJECTS += $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay_084.c.o
