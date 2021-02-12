#pragma once

#include <gtkmm.h>
#include <giomm/inputstream.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>

class LogView {
public:
	LogView();
	Gtk::Widget &operator()() { return window_; }

	void load(Gio::InputStream &is);

private:
	void reset();
	void update();

	void onScroll();
	void onResize(Gdk::Rectangle &rect);

	int pixelsPerLine_ = 20;
	int maxWidth_ = 0;

	Gtk::ScrolledWindow window_;
	Gtk::Fixed container_;

	std::unordered_map<size_t, std::unique_ptr<Gtk::Label>> widgets_;

	std::vector<char> input_;
	std::vector<char *> inputLines_;
};
