#include <gtkmm.h>
#include <gtkmm/application.h>
#include <vector>
#include <memory>

#include "PatternEditor.h"
#include "LogView.h"
#include "Pattern.h"
#include "log.h"

class MainWindow: public Gtk::Window {
public:
	MainWindow();

	void load(Gio::InputStream &stream) { logView_.load(stream); }

private:
	void onNewPatterns(std::vector<std::shared_ptr<Pattern>> patterns);
	void onPatternsUpdated();

	LogView logView_{Gdk::RGBA{"rgba(255, 255, 255, 1)"}, Gdk::RGBA{"rgba(0, 0, 0, 1)"}};
	PatternEditor patternEditor_;
	Gtk::Box mainBox_{Gtk::ORIENTATION_HORIZONTAL};
};

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

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("coffee.mort.logadogg");

	MainWindow window;
	window.set_default_size(1000, 600);

	return app->run(window);
}
