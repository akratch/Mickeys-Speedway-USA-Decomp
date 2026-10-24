# Overlay 61 owns one complete source module, including its genuine name table.
# Preserve the compiler object and use the ordinary section-relative module link.
MODULE_LINKED_OVERLAYS += 061
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
