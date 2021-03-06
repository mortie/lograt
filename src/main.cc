#include <gtkmm.h>
#include <gtkmm/application.h>
#include <vector>
#include <memory>
#include <giomm.h>

#include "MainWindow.h"
#include "log.h"

int main(int argc, char* argv[]) {
	auto app = Gtk::Application::create("coffee.mort.lograt");

	MainWindow window;
	window.set_default_icon_name("lograt");
	window.set_default_size(1000, 600);

#ifdef __unix__
	if (argc == 1) {
		auto stream = Gio::UnixInputStream::create(0, false);
		window.load(std::move(stream));
	} else if (argc >= 2) {
#else
	if (argc >= 2) {
#endif
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

	app->set_accel_for_action("app.open", "<Control>o");
	app->add_action("open", [&]() {
		window.showFilePicker();
	});

	return app->run(window);
}
