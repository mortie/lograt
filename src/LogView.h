#pragma once

#include <gtkmm.h>
#include <giomm/inputstream.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>

#include "Pattern.h"

class LogLine: public Gtk::Widget {
public:
	LogLine(const char *text, int width, int height, Color bg, Color fg);

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
	Color bg_, fg_;
	Glib::RefPtr<Pango::Layout> layout_;
};

class LogView {
public:
	LogView(Color bg, Color fg);
	Gtk::Widget &operator()() { return window_; }

	void load(Gio::InputStream &is);
	void setColors(Color bg, Color fg);
	void setPatterns(std::vector<std::shared_ptr<Pattern>> patterns);

private:
	void reset();
	void update();

	std::unique_ptr<LogLine> makeWidget(size_t line);

	void onScroll();
	void onResize(Gdk::Rectangle &rect);

	int pixelsPerLine_ = 20;
	int maxWidth_ = 0;
	Color bg_, fg_;
	std::vector<std::shared_ptr<Pattern>> patterns_;

	Gtk::ScrolledWindow window_;
	Gtk::Fixed container_;

	std::unordered_map<size_t, std::unique_ptr<LogLine>> widgets_;

	std::vector<char> input_;
	std::vector<char *> inputLines_;
};
