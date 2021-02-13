#include <gtkmm.h>
#include <gtkmm/application.h>
#include <vector>
#include <memory>

#include "PatternEditor.h"
#include "LogView.h"
#include "Pattern.h"

class MainWindow: public Gtk::Window {
public:
	MainWindow();

	void load(Gio::InputStream &stream) { logView_.load(stream); }

private:
	LogView logView_{{1, 1, 1}, {0, 0, 0}};
	PatternEditor patternEditor_;
	Gtk::Box mainBox_{Gtk::ORIENTATION_HORIZONTAL};
};

MainWindow::MainWindow() {
	set_title("LogaDogg");
	set_default_size(200, 200);

	std::vector<std::shared_ptr<Pattern>> patterns;
	patterns.push_back(std::make_shared<Pattern>(
			"feature", Color{0.5, 0.5, 0.5}, Color{1, 1, 1}));
	patterns.back()->compile();
	logView_.setPatterns(std::move(patterns));

	logView_().set_hexpand(true);
	mainBox_.add(logView_());

	patternEditor_().set_size_request(200, -1);
	mainBox_.add(patternEditor_());

	auto stdinput = Gio::UnixInputStream::create(0, false);
	load(*stdinput.get());
	stdinput.reset();

	add(mainBox_);
	show_all_children();
}

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("coffee.mort.logadogg");

	MainWindow window;
	window.set_default_size(640, 480);

	return app->run(window);
}
