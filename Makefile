BUILD := build

DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATAROOTDIR ?= $(PREFIX)/share

SRCS := $(wildcard src/*.cc)
HDRS := $(wildcard src/*.h)
OBJS := $(patsubst %,$(BUILD)/%.o,$(SRCS))

PKGS := gtkmm-3.0
FLAGS := -Wall -Wextra -Wno-unused-parameter -Wpedantic -g -std=c++17 \
	$(shell pkg-config --cflags $(PKGS))
LDLIBS := -lpcre2-8 \
	$(shell pkg-config --libs $(PKGS))

$(BUILD)/lograt: $(OBJS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(OBJS) $(LDLIBS)

$(BUILD)/%.o: % $(HDRS)
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(BUILD)

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
