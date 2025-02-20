#include <gtkmm.h>
#include <gtkmm/application.h>
#include <giomm.h>

#include "MainWindow.h"
#include "log.h"

#ifdef __APPLE__
#define HOTKEY_MODIFIER "<Meta>"
#else
#define HOTKEY_MODIFIER "<Control>"
#endif

int main(int argc, char* argv[]) {
	auto app = Gtk::Application::create("coffee.mort.lograt");

	/*
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
			Gtk::MessageDialog dialog("Open file failed", false, Gtk::MessageType::ERROR);
			dialog.set_secondary_text(err.what());
			dialog.error_bell();
			//dialog.run();
			return 1;
		}
	}
	*/

	app->set_accel_for_action("app.open", HOTKEY_MODIFIER "o");
	app->add_action("open", [&] {
		auto *win = static_cast<MainWindow *>(app->get_run_window());
		win->showFilePicker();
	});

	app->add_action("quit", [&] {
		app->quit();
	});

	return app->make_window_and_run<MainWindow>(argc, argv);
}
