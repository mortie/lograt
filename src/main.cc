#include <gtkmm.h>
#include <gtkmm/application.h>
#include <vector>
#include <memory>

#include "MainWindow.h"

int main(int argc, char* argv[])
{
	auto app = Gtk::Application::create("coffee.mort.logadogg");

	MainWindow window;
	window.set_default_size(1000, 600);

	return app->run(window);
}
