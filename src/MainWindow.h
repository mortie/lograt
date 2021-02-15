#pragma once

#include <gtkmm.h>
#include <vector>

#include "PatternEditor.h"
#include "LogView.h"
#include "Pattern.h"

class MainWindow: public Gtk::Window {
public:
	MainWindow();

	void load(Gio::InputStream &stream);

private:
	void onNewPatterns(std::vector<std::shared_ptr<Pattern>> patterns);
	void onPatternsUpdated();

	void onOpenButton();

	LogView logView_{Gdk::RGBA{"rgba(255, 255, 255, 1)"}, Gdk::RGBA{"rgba(0, 0, 0, 1)"}};
	PatternEditor patternEditor_;
	Gtk::Paned mainBox_{Gtk::ORIENTATION_HORIZONTAL};

	Gtk::HeaderBar headerBar_;
	Gtk::Button openButton_{"Open"};
};
