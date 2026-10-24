# Overlay 65 retains a separate initializer because its current peeled loop
# still needs unrolling disabled. The five-function module body uses ordinary
# overlay flags, its final alignment supplies the reviewed module-end pad,
# and the generated surface owns all resident call identities.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0

# Complete initialized effect data from actual typed triangle/color/point
# records and mutable resource/state cells. No instruction normalization.
O65_EFFECT_DATA := $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65EffectData.c.o
$(O65_EFFECT_DATA): private PLAIN_DATA_PROOF = 1
$(O65_EFFECT_DATA): private PLAIN_DATA_RUNTIME_ROLE = data
