OUT ?= build

PROGRAM = lograt

.PHONY: build
build: $(OUT)/build.ninja
	ninja -C $(OUT) $(PROGRAM)

.PHONY: setup
setup: $(OUT)/build.ninja

$(OUT)/build.ninja:
	meson setup $(OUT)

.PHONY: run
run: build
	cd $(OUT) && $(CMD) ./$(PROGRAM)

.PHONY: clean
clean:
	ninja -C $(OUT) clean

.PHONY: cleanall
cleanall:
	rm -rf $(OUT)
