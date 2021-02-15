#include "MainWindow.h"

#include "log.h"

MainWindow::MainWindow() {
	set_title("LogaDogg");
	set_default_size(200, 200);

	openButton_.signal_clicked().connect(
			sigc::mem_fun(this, &MainWindow::onOpenButton));

	headerBar_.set_title("LogaDogg");
	headerBar_.set_show_close_button(true);
	headerBar_.add(openButton_);
	set_titlebar(headerBar_);

	logView_().set_hexpand(true);
	mainBox_.pack1(logView_(), true, false);

	patternEditor_().set_size_request(200, -1);
	mainBox_.pack2(patternEditor_(), false, false);

	add(mainBox_);
	show_all_children();

	patternEditor_.signalNewPatterns().connect(
			sigc::mem_fun(this, &MainWindow::onNewPatterns));
	patternEditor_.signalPatternsUpdated().connect(
			sigc::mem_fun(this, &MainWindow::onPatternsUpdated));
}

void MainWindow::load(Gio::InputStream &stream) {
	try {
		logView_.load(stream);
	} catch (Gio::Error &err) {
		logln(err.what());
		Gtk::MessageDialog dialog("Open file failed", false, Gtk::MESSAGE_ERROR);
		dialog.set_secondary_text(err.what());
		dialog.error_bell();
		dialog.run();
	}
}

void MainWindow::onNewPatterns(std::vector<std::shared_ptr<Pattern>> patterns) {
	logView_.setPatterns(std::move(patterns));
}

void MainWindow::onPatternsUpdated() {
	logView_.patternsUpdated();
}

void MainWindow::onOpenButton() {
	auto chooser = Gtk::FileChooserNative::create(
			"Open Log File", Gtk::FILE_CHOOSER_ACTION_OPEN);
	auto res = chooser->run();
	if (res == Gtk::RESPONSE_ACCEPT) {
		auto file = chooser->get_file();
		auto stream = file->read();
		load(*stream.get());
	}
}
