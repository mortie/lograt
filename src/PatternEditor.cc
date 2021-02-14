#include "PatternEditor.h"

#include "log.h"

static const char *bgColors[] = {
	"#ff9c85",
	"#b5be00",
	"#00ce9e",
	"#00c6f4",
	"#e09cff",
	"#ff98ae",
	"#d8b300",
	"#00d25f",
	"#00c9d6",
	"#b7acff",
	"#ff94cc",
	"#fba300",
	"#82c900",
	"#00ccbd",
	"#81b9ff",
	"#ff8df0",
};

static const char *fgColor = "#000000";

static Gdk::RGBA makeColor(const char *str) {
	Gdk::RGBA rgba{str};
	rgba.set_alpha(1);
	return rgba;
}

PatternEditor::PatternEditor() {
	newPatternBackground_.set_hexpand(true);
	newPatternBackground_.set_tooltip_text("Background Color");
	newPatternForeground_.set_hexpand(true);
	newPatternForeground_.set_tooltip_text("Foreground Color");
	newPatternColorBox_.add(newPatternBackground_);
	newPatternColorBox_.add(newPatternForeground_);

	newPatternBackground_.set_rgba(makeColor(bgColors[0]));
	newPatternForeground_.set_rgba(makeColor(fgColor));

	newPatternRx_.set_placeholder_text("Pattern RegEx");
	newPatternBox_.add(newPatternRx_);
	newPatternBox_.add(newPatternColorBox_);
	newPatternBox_.add(newPatternAdd_);

	newPatternFrame_.add(newPatternBox_);
	newPatternFrame_.set_margin_bottom(10);

	container_.add(newPatternFrame_);
	container_.set_hexpand(false);

	window_.add(container_);
	window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

	newPatternRx_.signal_activate().connect(
			sigc::mem_fun(this, &PatternEditor::onPatternSubmit));
	newPatternAdd_.signal_clicked().connect(
			sigc::mem_fun(this, &PatternEditor::onPatternSubmit));
}

PatternEditor::PatternBox::PatternBox(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg):
		pattern(std::make_shared<Pattern>(rx, bg, fg)) {}

std::unique_ptr<PatternEditor::PatternBox> PatternEditor::makePatternBox(
		const char *rx, Gdk::RGBA bg, Gdk::RGBA fg) {
	auto pat = std::make_unique<PatternBox>(rx, bg, fg);

	auto onChange = [pat = pat.get(), this]() { onPatternChanged(pat); };

	std::string err = pat->pattern->compile();
	if (err.size() > 0) {
		pat->error.set_text(err);
	}
	pat->error.set_line_wrap(true);
	pat->error.set_margin_top(5);
	pat->error.set_margin_bottom(5);
	pat->error.set_margin_left(5);
	pat->error.set_margin_right(5);

	pat->box.add(pat->error);
	pat->patternRx.set_text(rx);
	pat->patternRx.signal_activate().connect(onChange);
	pat->patternRx.signal_changed().connect(onChange);
	pat->box.add(pat->patternRx);

	pat->background.set_rgba(bg);
	pat->background.set_hexpand(true);
	pat->background.signal_color_set().connect(onChange);
	pat->foreground.set_rgba(fg);
	pat->foreground.set_hexpand(true);
	pat->foreground.signal_color_set().connect(onChange);
	pat->colorBox.add(pat->background);
	pat->colorBox.add(pat->foreground);
	pat->box.add(pat->colorBox);

	pat->deleteButton.set_hexpand(true);
	pat->deleteButton.signal_clicked().connect([pat = pat.get(), this]() {
		deletePattern(pat);
	});
	pat->actionBox.add(pat->deleteButton);

	pat->upButton.set_hexpand(true);
	pat->upButton.signal_clicked().connect([pat = pat.get(), this]() {
		movePattern(pat, -1);
	});
	pat->actionBox.add(pat->upButton);

	pat->downButton.set_hexpand(true);
	pat->downButton.signal_clicked().connect([pat = pat.get(), this]() {
		movePattern(pat, 1);
	});
	pat->actionBox.add(pat->downButton);

	pat->box.add(pat->actionBox);

	pat->box.set_hexpand(false);
	pat->frame.add(pat->box);
	pat->frame.show_all();
	pat->frame.set_margin_top(10);
	pat->frame.set_margin_bottom(10);

	if (err.size() == 0) {
		pat->error.hide();
	}

	return pat;
}

void PatternEditor::resetNewPattern() {
	newPatternRx_.set_text("");
}

void PatternEditor::emitCurrentPatterns() {
	std::vector<std::shared_ptr<Pattern>> patterns;
	patterns.reserve(patterns_.size());
	for (auto &box: patterns_) {
		patterns.push_back(box->pattern);
	}
	signalNewPatterns_.emit(std::move(patterns));
}

void PatternEditor::deletePattern(PatternBox *box) {
	auto it = patterns_.begin();
	while (it != patterns_.end()) {
		if (it->get() == box) {
			break;
		}

		++it;
	}

	if (it == patterns_.end()) {
		return;
	}

	container_.remove(box->box);
	patterns_.erase(it);
	emitCurrentPatterns();
}

void PatternEditor::movePattern(PatternBox *fromBox, int direction) {
	auto fromIt = patterns_.begin();
	while (fromIt != patterns_.end()) {
		if (fromIt->get() == fromBox) {
			break;
		}

		++fromIt;
	}

	if (fromIt == patterns_.end()) {
		return;
	}

	if (
			(direction == -1 && fromIt == patterns_.begin()) ||
			(direction == 1 && fromIt == patterns_.end() - 1)) {
		return;
	}

	auto toIt = fromIt + (direction < 0 ? -1 : 1);

	gint fromIdx = fromIt - patterns_.begin();
	if (direction < 0) {
		logln("Moving child to " << (fromIdx));
		container_.reorder_child((*fromBox)(), fromIdx);
	} else {
		logln("Moving child to " << (fromIdx + 2));
		container_.reorder_child((*fromBox)(), fromIdx + 2);
	}

	fromIt->swap(*toIt);
	emitCurrentPatterns();
}

void PatternEditor::onPatternSubmit() {
	Glib::ustring rx = newPatternRx_.get_text();
	if (rx.size() == 0) {
		return;
	}

	auto bg = newPatternBackground_.get_rgba();
	auto fg = newPatternForeground_.get_rgba();
	patterns_.push_back(makePatternBox(rx.c_str(), bg, fg));
	container_.add((*patterns_.back())());

	emitCurrentPatterns();

	colorIndex_ = (colorIndex_ + 1) % (sizeof(bgColors) / sizeof(*bgColors));
	newPatternBackground_.set_rgba(makeColor(bgColors[colorIndex_]));

	resetNewPattern();
}

void PatternEditor::onPatternChanged(PatternBox *box) {
	Pattern pat(box->patternRx.get_text().c_str(), box->pattern->bg_, box->pattern->fg_);
	std::string err = pat.compile();
	if (err.size() > 0) {
		box->error.set_text(err.c_str());
		box->error.show();
	} else {
		box->error.hide();
	}

	pat.bg_ = box->background.get_rgba();
	pat.fg_ = box->foreground.get_rgba();

	*box->pattern = std::move(pat);
	signalPatternsUpdated_.emit();
}
