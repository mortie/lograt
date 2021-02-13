#include <gtkmm.h>
#include <vector>

#include "Pattern.h"

class SinglePatternEditor {
public:
	SinglePatternEditor();
	Gtk::Widget &operator()() { return box_; }

private:
	Gtk::Box box_{Gtk::ORIENTATION_VERTICAL};
	Gtk::Entry rx_;
	Gtk::Button addBtn_{"Add"};
};

class PatternEditor {
public:
	PatternEditor();
	Gtk::Widget &operator()() { return container_; }

private:
	void resetNewPattern();

	void onPatternSubmit();

	Gtk::Box container_{Gtk::ORIENTATION_VERTICAL};

	std::vector<SinglePatternEditor> patterns_;

	Gtk::Box newPatternBox_{Gtk::ORIENTATION_VERTICAL};
	Gtk::Entry newPatternRx_;
	Gtk::Box newPatternColorBox_{Gtk::ORIENTATION_HORIZONTAL};
	Gtk::ColorButton newPatternBackground_{Gdk::RGBA{"rgba(255, 255, 255, 1)"}};
	Gtk::ColorButton newPatternForeground_{Gdk::RGBA{"rgba(0, 0, 0, 1)"}};
	Gtk::Button newPatternAdd_{"Add"};
	Gtk::Separator separator_;
};
