#include "PatternEditor.h"

#include "log.h"

PatternEditor::PatternEditor() {
	newPatternBackground_.set_hexpand(true);
	newPatternBackground_.set_tooltip_text("Background Color");
	newPatternForeground_.set_hexpand(true);
	newPatternForeground_.set_tooltip_text("Foreground Color");
	newPatternColorBox_.add(newPatternBackground_);
	newPatternColorBox_.add(newPatternForeground_);

	newPatternRx_.set_placeholder_text("Pattern RegEx");
	newPatternBox_.add(newPatternRx_);
	newPatternBox_.add(newPatternColorBox_);
	newPatternBox_.add(newPatternAdd_);

	newPatternFrame_.add(newPatternBox_);
	newPatternFrame_.set_margin_bottom(10);

	container_.add(newPatternFrame_);
	container_.set_hexpand(false);

	window_.add(container_);

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

	std::string err = pat->pattern->compile();
	if (err.size() > 0) {
		pat->error.set_text(err);
	} else {
		pat->error.hide();
	}

	auto onChange = [pat = pat.get(), this]() { onPatternChanged(pat); };

	pat->box.add(pat->error);
	pat->box.show_all();
	pat->patternRx.set_text(rx);
	pat->patternRx.signal_activate().connect(onChange);
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
	pat->box.set_hexpand(false);

	pat->frame.add(pat->box);
	pat->frame.show_all();
	pat->frame.set_margin_top(10);
	pat->frame.set_margin_bottom(10);
	return pat;
}

void PatternEditor::resetNewPattern() {
	newPatternRx_.set_text("");
}

void PatternEditor::onPatternSubmit() {
	Glib::ustring rx = newPatternRx_.get_text();
	if (rx.size() == 0) {
		return;
	}

	auto bg = newPatternBackground_.get_rgba();
	auto fg = newPatternForeground_.get_rgba();
	patterns_.push_back(makePatternBox(rx.c_str(), bg, fg));
	container_.add(patterns_.back()->frame);

	std::vector<std::shared_ptr<Pattern>> patterns;
	patterns.reserve(patterns_.size());
	for (auto &box: patterns_) {
		patterns.push_back(box->pattern);
	}
	signalNewPatterns_.emit(std::move(patterns));

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
