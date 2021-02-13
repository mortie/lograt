#include <gtkmm.h>
#include <gtkmm/application.h>

#include "LogView.h"

class MainWindow: public Gtk::Window {
public:
	MainWindow();

	void load(Gio::InputStream &stream) { logView_.load(stream); }

private:
	LogView logView_;
};

MainWindow::MainWindow() {
	set_title("LogaDogg");
	set_default_size(200, 200);

	add(logView_());

	auto stdinput = Gio::UnixInputStream::create(0, false);
	load(*stdinput.get());
	stdinput.reset();

	show_all_children();
}

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("coffee.mort.logadogg");

	MainWindow window;
	window.set_default_size(640, 480);

	return app->run(window);
}
