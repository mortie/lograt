#include "MainWindow.h"

#include "log.h"

MainWindow::MainWindow() {
	set_title("Lograt");
	set_default_size(1000, 600);
	set_default_icon_name("coffee.mort.lograt");

	openButton_.signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::showFilePicker));

	//headerBar_.set_title("Lograt");
	//headerBar_.set_show_close_button(true);
	headerBar_.pack_start(openButton_);
	set_titlebar(headerBar_);

	logView_().set_hexpand(true);
	mainBox_.set_start_child(logView_());
	mainBox_.set_resize_start_child(true);
	mainBox_.set_shrink_start_child(false);

	patternEditor_().set_size_request(200, -1);
	mainBox_.set_end_child(patternEditor_());
	mainBox_.set_resize_end_child(false);
	mainBox_.set_shrink_end_child(false);

	set_child(mainBox_);

	patternEditor_.signalNewPatterns().connect(
			sigc::mem_fun(*this, &MainWindow::onNewPatterns));
	patternEditor_.signalPatternsUpdated().connect(
			sigc::mem_fun(*this, &MainWindow::onPatternsUpdated));
	patternEditor_.signalSearch().connect(
			sigc::mem_fun(*this, &MainWindow::onSearch));
	patternEditor_.signalUnsearch().connect(
			sigc::mem_fun(*this, &MainWindow::onUnsearch));
}

void MainWindow::load(Glib::RefPtr<Gio::InputStream> stream) {
	try {
		logView_.load(std::move(stream));
	} catch (Gio::Error &err) {
		logln(err.what());
		Gtk::MessageDialog dialog("Open file failed", false, Gtk::MessageType::ERROR);
		dialog.set_transient_for(*this);
		dialog.set_secondary_text(err.what());
		dialog.error_bell();
		dialog.show();
	}
}

void MainWindow::showFilePicker() {
	auto chooser = Gtk::FileChooserNative::create(
			"Open Log File", Gtk::FileChooser::Action::OPEN);
	if (prevOpenDirUri_.size() > 0) {
		chooser->set_current_folder(Gio::File::create_for_uri(prevOpenDirUri_));
	}

	chooser->signal_response().connect([chooser, this](int res) {
		if (res != Gtk::ResponseType::ACCEPT) {
			return;
		}

		auto file = chooser->get_file();
		auto stream = file->read();
		load(std::move(stream));
	});

	chooser->set_transient_for(*this);
	chooser->show();
}

void MainWindow::onNewPatterns(std::vector<std::shared_ptr<Pattern>> patterns) {
	logView_.setPatterns(std::move(patterns));
}

void MainWindow::onPatternsUpdated() {
	logView_.patternsUpdated();
}

void MainWindow::onSearch(std::shared_ptr<Pattern> pattern) {
	logView_.search(std::move(pattern));
}

void MainWindow::onUnsearch() {
	logView_.unsearch();
}
