#include <gtkmm.h>
#include <gtkmm/application.h>
#include <vector>
#include <memory>
#include <giomm.h>

#include "MainWindow.h"
#include "log.h"

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("coffee.mort.lograt");

	MainWindow window;
	window.set_default_icon_name("lograt");
	window.set_default_size(1000, 600);

	if (argc == 2) {
#ifdef __unix__
		if (strcmp(argv[1], "-") == 0) {
			auto stream = Gio::UnixInputStream::create(0, false);
			window.load(std::move(stream));
		} else
#endif
		{
			auto file = Gio::File::create_for_path(argv[1]);
			try {
				auto stream = file->read();
				window.load(std::move(stream));
			} catch (Gio::Error &err) {
				logln(err.what());
				Gtk::MessageDialog dialog("Open file failed", false, Gtk::MESSAGE_ERROR);
				dialog.set_secondary_text(err.what());
				dialog.error_bell();
				dialog.run();
				return 1;
			}
		}
	}

	return app->run(window);
}
