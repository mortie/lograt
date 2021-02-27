#pragma once

#include <gtkmm.h>
#include <giomm/inputstream.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <type_traits>

#include "Pattern.h"

class LogLine: public Gtk::Widget {
public:
	LogLine(const char *text, int height, Gdk::RGBA bg, Gdk::RGBA fg);

	void setHighlighted(bool hl);

protected:
	void get_preferred_width_vfunc(int &min, int &nat) const final override;
	void get_preferred_width_for_height_vfunc(int h, int &min, int &nat) const final override;
	void get_preferred_height_vfunc(int &min, int &nat) const final override;
	void get_preferred_height_for_width_vfunc(int w, int &min, int &nat) const final override;
	void on_size_allocate(Gtk::Allocation &allocation) final override;
	void on_realize() final override;
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) final override;

private:
	static constexpr int HPADDING = 4;
	const char *text_;
	int height_;
	Gdk::RGBA bg_, fg_;
	bool isHighlighted_ = false;
	Glib::RefPtr<Pango::Layout> layout_;
};

class LogView {
public:
	LogView(Gdk::RGBA bg, Gdk::RGBA fg);
	Gtk::Widget &operator()() { return paned_; }

	void load(Glib::RefPtr<Gio::InputStream> is);
	void setColors(Gdk::RGBA bg, Gdk::RGBA fg);
	void setPatterns(std::vector<std::shared_ptr<Pattern>> patterns);
	void patternsUpdated();
	void search(std::shared_ptr<Pattern> pattern);
	void unsearch();

private:
	struct SearchResult {
		LogLine widget;
		size_t lineNum;
	};

	void reset();
	void update();

	std::unique_ptr<LogLine> makeWidget(size_t line);

	void onScroll();
	void onResize(Gdk::Rectangle &rect);

	Gtk::Paned paned_{Gtk::ORIENTATION_VERTICAL};

	int pixelsPerLine_ = 20;
	int maxWidth_ = 0;
	ssize_t highlightedLine_ = -1;
	ssize_t highlightedSearchResult_ = -1;
	Gdk::RGBA bg_, fg_;
	std::shared_ptr<Pattern> searchPattern_;
	std::vector<std::shared_ptr<Pattern>> patterns_;

	Gtk::ScrolledWindow window_;
	Gtk::Fixed container_;

	std::vector<SearchResult> searchResults_;
	Gtk::ScrolledWindow searchWindow_;
	Gtk::Fixed searchContainer_;

	std::unordered_map<size_t, std::unique_ptr<LogLine>> widgets_;

	std::recursive_mutex mut_;
	std::vector<char> input_;
	std::vector<size_t> inputLines_;
};
