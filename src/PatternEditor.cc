#include "PatternEditor.h"

#include <gdkmm.h>

#include "colors.h"

PatternEditor::PatternEditor() {
	newPatternBackground_.set_hexpand(true);
	newPatternBackground_.set_tooltip_text("Background Color");
	newPatternForeground_.set_hexpand(true);
	newPatternForeground_.set_tooltip_text("Foreground Color");
	newPatternColorBox_.append(newPatternBackground_);
	newPatternColorBox_.append(newPatternForeground_);

	newPatternBackground_.set_rgba(patternBgColors[0]);
	newPatternForeground_.set_rgba(textFgColor);

	newPatternRx_.set_placeholder_text("Pattern RegEx");
	newPatternRx_.set_icon_from_icon_name("search", Gtk::Entry::IconPosition::SECONDARY);
	newPatternBox_.append(newPatternRx_);
	newPatternBox_.append(newPatternColorBox_);
	newPatternBox_.append(newPatternAdd_);

	newPatternFrame_.set_child(newPatternBox_);
	newPatternFrame_.set_margin_bottom(10);

	container_.append(newPatternFrame_);
	container_.set_hexpand(false);

	window_.set_child(container_);
	window_.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);

	newPatternRx_.signal_activate().connect(
			sigc::mem_fun(*this, &PatternEditor::onPatternSubmit));
	newPatternRx_.signal_icon_press().connect(
			sigc::mem_fun(*this, &PatternEditor::onSearchClicked));
	newPatternAdd_.signal_clicked().connect(
			sigc::mem_fun(*this, &PatternEditor::onPatternSubmit));

	auto rxEntryController = Gtk::EventControllerKey::create();
	rxEntryController->signal_key_pressed().connect(
			sigc::mem_fun(*this, &PatternEditor::onRxKeyPress), false);

	newPatternRx_.add_controller(std::move(rxEntryController));
}

PatternEditor::PatternBox::PatternBox(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg):
		pattern(std::make_shared<Pattern>(rx, bg, fg)) {}

std::unique_ptr<PatternEditor::PatternBox> PatternEditor::makePatternBox(
		const char *rx, Gdk::RGBA bg, Gdk::RGBA fg) {
	auto pat = std::make_unique<PatternBox>(rx, bg, fg);

	auto onChange = [pat = pat.get(), this]() { onPatternChanged(pat); };

	auto onSearchClicked = [pat = pat.get(), this](Gtk::Entry::IconPosition pos) {
		emitSearch(
				pat->patternRx.get_text().c_str(),
				pat->pattern->bg_, pat->pattern->fg_);
	};

	auto onKeyPress = [pat = pat.get(), this](guint keyval, guint keycode, Gdk::ModifierType state) {
		bool ctrl = (state & Gdk::ModifierType::CONTROL_MASK) != Gdk::ModifierType::NO_MODIFIER_MASK;
		if (keyval == GDK_KEY_Return && ctrl) {
			emitSearch(
					pat->patternRx.get_text().c_str(),
					pat->pattern->bg_, pat->pattern->fg_);
			return true;
		}

		return false;
	};

	std::string err = pat->pattern->compile();
	if (err.size() > 0) {
		pat->error.set_text(err);
	}
	//pat->error.set_line_wrap(true);
	pat->error.set_margin_top(5);
	pat->error.set_margin_bottom(5);
	//pat->error.set_margin_left(5);
	//pat->error.set_margin_right(5);

	auto rxEntryController = Gtk::EventControllerKey::create();
	rxEntryController->signal_key_pressed().connect(onKeyPress, false);

	pat->box.append(pat->error);
	pat->patternRx.set_text(rx);
	pat->patternRx.signal_activate().connect(onChange);
	pat->patternRx.signal_changed().connect(onChange);
	pat->patternRx.signal_icon_press().connect(onSearchClicked);
	pat->patternRx.add_controller(rxEntryController);
	pat->patternRx.set_icon_from_icon_name("search", Gtk::Entry::IconPosition::SECONDARY);
	pat->box.append(pat->patternRx);

	pat->background.set_rgba(bg);
	pat->background.set_hexpand(true);
	pat->background.signal_color_set().connect(onChange);
	pat->foreground.set_rgba(fg);
	pat->foreground.set_hexpand(true);
	pat->foreground.signal_color_set().connect(onChange);
	pat->colorBox.append(pat->background);
	pat->colorBox.append(pat->foreground);
	pat->box.append(pat->colorBox);

	pat->deleteButton.set_hexpand(true);
	pat->deleteButton.signal_clicked().connect([pat = pat.get(), this]() {
		deletePattern(pat);
	});
	pat->actionBox.append(pat->deleteButton);

	pat->upButton.set_hexpand(true);
	pat->upButton.signal_clicked().connect([pat = pat.get(), this]() {
		movePattern(pat, -1);
	});
	pat->actionBox.append(pat->upButton);

	pat->downButton.set_hexpand(true);
	pat->downButton.signal_clicked().connect([pat = pat.get(), this]() {
		movePattern(pat, 1);
	});
	pat->actionBox.append(pat->downButton);

	pat->box.append(pat->actionBox);

	pat->box.set_hexpand(false);
	pat->frame.set_child(pat->box);
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
		container_.reorder_child_after((*fromBox)(), (*patterns_[fromIdx])());
	} else {
		container_.reorder_child_after((*fromBox)(), (*patterns_[fromIdx + 2])());
	}

	fromIt->swap(*toIt);
	emitCurrentPatterns();
}

void PatternEditor::emitSearch(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg) {
	if (rx[0] == '\0') {
		signalUnsearch_.emit();
		return;
	}

	auto pattern = std::make_shared<Pattern>(rx, bg, fg);
	std::string error = pattern->compile();
	if (error.size() > 0) {
		Gtk::MessageDialog dialog("Search regex error", false, Gtk::MessageType::ERROR);
		dialog.set_secondary_text(error);
		dialog.error_bell();
		//dialog.run();
		return;
	}

	signalSearch_.emit(std::move(pattern));
}

void PatternEditor::onPatternSubmit() {
	Glib::ustring rx = newPatternRx_.get_text();
	if (rx.size() == 0) {
		return;
	}

	auto bg = newPatternBackground_.get_rgba();
	auto fg = newPatternForeground_.get_rgba();
	patterns_.push_back(makePatternBox(rx.c_str(), bg, fg));
	container_.append((*patterns_.back())());

	emitCurrentPatterns();

	colorIndex_ = (colorIndex_ + 1) % patternBgColorsLen;
	newPatternBackground_.set_rgba(patternBgColors[colorIndex_]);

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

void PatternEditor::onSearchClicked(Gtk::Entry::IconPosition pos) {
	emitSearch(
			newPatternRx_.get_text().c_str(),
			newPatternBackground_.get_rgba(),
			newPatternForeground_.get_rgba());
}

bool PatternEditor::onRxKeyPress(guint keyval, guint keycode, Gdk::ModifierType state) {
	bool ctrl = (state & Gdk::ModifierType::CONTROL_MASK) != Gdk::ModifierType::NO_MODIFIER_MASK;
	if (keyval == GDK_KEY_Return && ctrl) {
		emitSearch(
				newPatternRx_.get_text().c_str(),
				newPatternBackground_.get_rgba(),
				newPatternForeground_.get_rgba());
		return true;
	}

	return false;
}
