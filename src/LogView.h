#pragma once

#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <giomm/inputstream.h>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Pattern.h"

class LogLine {
public:
	LogLine(const char *text, int height, Gdk::RGBA bg, Gdk::RGBA fg);

	void setHighlighted(bool hl);
	Gtk::DrawingArea &widget() { return area_; }

private:
	void draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);

	static constexpr int HPADDING = 4;
	const char *text_;
	int height_;
	Gdk::RGBA bg_, fg_;
	bool isHighlighted_ = false;
	Glib::RefPtr<Pango::Layout> layout_;
	Gtk::DrawingArea area_;
};

class LogView {
public:
	LogView(Gdk::RGBA bg, Gdk::RGBA fg);
	Gtk::Widget &operator()() { return paned_; }

	void load(Glib::RefPtr<Gio::InputStream> stream);
	void setPatterns(std::vector<std::shared_ptr<Pattern>> patterns);
	void patternsUpdated();
	void search(std::shared_ptr<Pattern> pattern);
	void unsearch();

private:
	struct SearchResult {
		LogLine line;
		size_t lineNum;
	};

	void reset();
	void update();

	std::unique_ptr<LogLine> makeLine(size_t line);

	void onScroll();
	void onLoadData(const Glib::RefPtr<Gio::AsyncResult> &result);

	Gtk::Paned paned_{Gtk::Orientation::VERTICAL};

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

	std::unordered_map<size_t, std::unique_ptr<LogLine>> lines_;

	struct LoadContext {
		Glib::RefPtr<Gio::Cancellable> cancelLoad;
		size_t index = 0;
		size_t startIndex = 0;
		static constexpr size_t BUFSIZE = 4096;
	} loadContext_;

	std::vector<char> input_;
	std::vector<size_t> inputLines_;
};
