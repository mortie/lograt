#pragma once

#include <gtkmm.h>
#include <giomm/inputstream.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>

class LogLine: public Gtk::Widget {
public:
	LogLine(const char *text, int width, int height);

protected:
	void get_preferred_width_vfunc(int &min, int &nat) const final override;
	void get_preferred_width_for_height_vfunc(int h, int &min, int &nat) const final override;
	void get_preferred_height_vfunc(int &min, int &nat) const final override;
	void get_preferred_height_for_width_vfunc(int w, int &min, int &nat) const final override;
	void on_size_allocate(Gtk::Allocation &allocation) final override;
	void on_realize() final override;
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) final override;

private:
	const char *text_;
	int width_, height_;
	Glib::RefPtr<Pango::Layout> layout_;
};

class LogView {
public:
	LogView();
	Gtk::Widget &operator()() { return window_; }

	void load(Gio::InputStream &is);

private:
	void reset();
	void update();

	std::unique_ptr<LogLine> makeWidget(size_t line);

	void onScroll();
	void onResize(Gdk::Rectangle &rect);

	int pixelsPerLine_ = 20;
	int maxWidth_ = 0;

	Gtk::ScrolledWindow window_;
	Gtk::Fixed container_;

	std::unordered_map<size_t, std::unique_ptr<LogLine>> widgets_;

	std::vector<char> input_;
	std::vector<char *> inputLines_;
};
