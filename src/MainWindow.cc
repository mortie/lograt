#include "MainWindow.h"

MainWindow::MainWindow() {
	set_title("LogaDogg");
	set_default_size(200, 200);

	logView_().set_hexpand(true);
	mainBox_.add(logView_());

	patternEditor_().set_size_request(200, -1);
	mainBox_.add(patternEditor_());

	auto stdinput = Gio::UnixInputStream::create(0, false);
	load(*stdinput.get());
	stdinput.reset();

	add(mainBox_);
	show_all_children();

	patternEditor_.signalNewPatterns().connect(
			sigc::mem_fun(this, &MainWindow::onNewPatterns));
	patternEditor_.signalPatternsUpdated().connect(
			sigc::mem_fun(this, &MainWindow::onPatternsUpdated));
}

void MainWindow::onNewPatterns(std::vector<std::shared_ptr<Pattern>> patterns) {
	logView_.setPatterns(std::move(patterns));
}

void MainWindow::onPatternsUpdated() {
	logView_.patternsUpdated();
}
