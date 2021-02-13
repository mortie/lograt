#include "PatternEditor.h"

#include "log.h"

SinglePatternEditor::SinglePatternEditor() {
	box_.add(rx_);
	box_.add(addBtn_);
}

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

	container_.add(newPatternBox_);
	container_.add(separator_);
	container_.set_hexpand(false);

	newPatternRx_.signal_activate().connect(
			sigc::mem_fun(this, &PatternEditor::onPatternSubmit));
	newPatternAdd_.signal_clicked().connect(
			sigc::mem_fun(this, &PatternEditor::onPatternSubmit));
}

void PatternEditor::resetNewPattern() {
	newPatternRx_.set_text("");
}

void PatternEditor::onPatternSubmit() {
	Glib::ustring rx = newPatternRx_.get_text();
	logln("Submitting pattern " << rx.c_str());
	resetNewPattern();
}
