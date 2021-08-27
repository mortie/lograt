SRCS = \
	src/LogView.cc \
	src/main.cc \
	src/MainWindow.cc \
	src/Pattern.cc \
	src/PatternEditor.cc \
#

PKGS := gtkmm-3.0 libpcre2-8
WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter
INCLUDES := -Isrc

FLAGS := $(WARNINGS) $(INCLUDES)
CXXFLAGS = $(FLAGS) -std=c++17
CFLAGS = $(FLAGS)
LDFLAGS :=
LDLIBS :=

HASH :=
OUT ?= build$(HASH)
CC ?= cc
CXX ?= c++
PKG_CONFIG ?= pkg-config

ifneq ($(PKGS),)
FLAGS += $(shell $(PKG_CONFIG) --cflags $(PKGS))
LDLIBS += $(shell $(PKG_CONFIG) --libs $(PKGS))
endif

ifneq ($(SANITIZE),)
HASH := $(HASH)-sanitize-$(SANITIZE)
LDFLAGS += -fsanitize=$(SANITIZE)
FLAGS += -fsanitize=$(SANITIZE)
endif

ifeq ($(RELEASE),1)
HASH := $(HASH)-release
FLAGS += -O2 -DNDEBUG
else
HASH := $(HASH)-debug
FLAGS += -g
endif

ifeq ($(VERBOSE),1)
define exec
	@echo '$(1):' $(2)
	@$(2)
endef
else
define exec
	@echo '$(1)' $@
	@$(2) || (echo '$(1) $@:' $(2) >&2 && false)
endef
endif

$(OUT)/target: $(patsubst %,$(OUT)/%.o,$(SRCS))
	@mkdir -p $(@D)
	$(call exec,LD ,$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS))

$(OUT)/%.cc.o: %.cc $(OUT)/%.cc.d
	@mkdir -p $(@D)
	$(call exec,CXX,$(CXX) $(CXXFLAGS) -MMD -o $@ -c $<)
$(OUT)/%.cc.d: %.cc;

$(OUT)/%.c.o: %.c $(OUT)/%.c.d
	@mkdir -p $(@D)
	$(call exec,CC ,$(CC) $(CFLAGS) -MMD -o $@ -c $<)
$(OUT)/%.c.d: %.c;

ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
-include $(patsubst %,$(OUT)/%.d,$(SRCS))
endif

.PHONY: clean
clean:
	rm -rf $(OUT)

.PHONY: install
install: $(BUILD)/lograt lograt.desktop icons/lograt.svg
	install -d $(DESTDIR)$(BINDIR)
	cp -f $(BUILD)/lograt $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(DATAROOTDIR)/applications/
	sed 's#@bindir@#$(BINDIR)#' lograt.desktop >$(DESTDIR)$(DATAROOTDIR)/applications/lograt.desktop
	install -d $(DESTDIR)$(DATAROOTDIR)/icons/hicolor/scalable/apps
	cp -f icons/lograt.svg $(DESTDIR)$(DATAROOTDIR)/icons/hicolor/scalable/apps/

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/lograt
	rm -f $(DESTDIR)$(DATAROOTDIR)/applications/lograt.desktop
	rm -f $(DESTDIR)$(DATAROOTDIR)/icons/scalable/apps/lograt.svg
