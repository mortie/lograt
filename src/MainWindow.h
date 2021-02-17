#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>

#include "colors.h"
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
	void onSearch(std::shared_ptr<Pattern> pattern);
	void onUnsearch();

	void onOpenButton();

	LogView logView_{textBgColor, textFgColor};
	PatternEditor patternEditor_;
	Gtk::Paned mainBox_{Gtk::ORIENTATION_HORIZONTAL};

	Gtk::HeaderBar headerBar_;
	Gtk::Button openButton_{"Open"};

	std::string prevOpenDirUri_;
};
