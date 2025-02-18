#pragma once

#include <vector>
#include <memory>
#include <gtkmm.h>
#include <glibmm/signalproxy.h>

#include "Pattern.h"

class PatternEditor {
public:
	PatternEditor();
	Gtk::Widget &operator()() { return window_; }

	auto &signalNewPatterns() { return signalNewPatterns_; }
	auto &signalPatternsUpdated() { return signalPatternsUpdated_; }
	auto &signalSearch() { return signalSearch_; }
	auto &signalUnsearch() { return signalUnsearch_; }

private:
	struct PatternBox {
		PatternBox(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg);
		Gtk::Widget &operator()() { return frame; }

		std::shared_ptr<Pattern> pattern;
		Gtk::Frame frame;
		Gtk::Box box{Gtk::Orientation::VERTICAL};
		Gtk::Label error;
		Gtk::Entry patternRx;
		Gtk::Box colorBox{Gtk::Orientation::HORIZONTAL};
		Gtk::ColorButton background;
		Gtk::ColorButton foreground;

		Gtk::Box actionBox{Gtk::Orientation::HORIZONTAL};
		Gtk::Button deleteButton{"X"};
		Gtk::Button upButton{"ᐃ"};
		Gtk::Button downButton{"ᐁ"};
	};

	std::unique_ptr<PatternBox> makePatternBox(
			const char *rx, Gdk::RGBA bg, Gdk::RGBA fg);
	void resetNewPattern();
	void emitCurrentPatterns();
	void deletePattern(PatternBox *box);
	void movePattern(PatternBox *box, int direction);
	void emitSearch(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg);

	void onPatternSubmit();
	void onPatternChanged(PatternBox *box);
	void onSearchClicked(Gtk::Entry::IconPosition pos);
	bool onRxKeyPress(guint keyval, guint keycode, Gdk::ModifierType state);

	std::vector<std::unique_ptr<PatternBox>> patterns_;
	Gtk::ScrolledWindow window_;
	Gtk::Box container_{Gtk::Orientation::VERTICAL};

	Gtk::Frame newPatternFrame_;
	Gtk::Box newPatternBox_{Gtk::Orientation::VERTICAL};
	Gtk::Entry newPatternRx_;
	Gtk::Box newPatternColorBox_{Gtk::Orientation::HORIZONTAL};
	Gtk::ColorButton newPatternBackground_{};
	Gtk::ColorButton newPatternForeground_{};
	Gtk::Button newPatternAdd_{"Add"};

	sigc::signal<void(std::vector<std::shared_ptr<Pattern>>)> signalNewPatterns_;
	sigc::signal<void()> signalPatternsUpdated_;
	sigc::signal<void(std::shared_ptr<Pattern>)> signalSearch_;
	sigc::signal<void()> signalUnsearch_;

	int colorIndex_ = 0;
};
