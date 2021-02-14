#include <gtkmm.h>
#include <vector>
#include <memory>
#include <glibmm/signalproxy.h>

#include "Pattern.h"

class PatternEditor {
public:
	PatternEditor();
	Gtk::Widget &operator()() { return window_; }

	auto &signalNewPatterns() { return signalNewPatterns_; }
	auto &signalPatternsUpdated() { return signalPatternsUpdated_; }

private:
	struct PatternBox {
		PatternBox(const char *rx, Gdk::RGBA bg, Gdk::RGBA fg);

		std::shared_ptr<Pattern> pattern;
		Gtk::Frame frame;
		Gtk::Box box{Gtk::ORIENTATION_VERTICAL};
		Gtk::Label error;
		Gtk::Entry patternRx;
		Gtk::Box colorBox{Gtk::ORIENTATION_HORIZONTAL};
		Gtk::ColorButton background;
		Gtk::ColorButton foreground;
	};

	std::unique_ptr<PatternBox> makePatternBox(
			const char *rx, Gdk::RGBA bg, Gdk::RGBA fg);
	void resetNewPattern();

	void onPatternSubmit();
	void onPatternChanged(PatternBox *box);

	std::vector<std::unique_ptr<PatternBox>> patterns_;
	Gtk::ScrolledWindow window_;
	Gtk::Box container_{Gtk::ORIENTATION_VERTICAL};

	Gtk::Frame newPatternFrame_;
	Gtk::Box newPatternBox_{Gtk::ORIENTATION_VERTICAL};
	Gtk::Entry newPatternRx_;
	Gtk::Box newPatternColorBox_{Gtk::ORIENTATION_HORIZONTAL};
	Gtk::ColorButton newPatternBackground_{Gdk::RGBA{"rgba(255, 255, 255, 1)"}};
	Gtk::ColorButton newPatternForeground_{Gdk::RGBA{"rgba(0, 0, 0, 1)"}};
	Gtk::Button newPatternAdd_{"Add"};

	sigc::signal<void(std::vector<std::shared_ptr<Pattern>>)> signalNewPatterns_;
	sigc::signal<void()> signalPatternsUpdated_;
};
