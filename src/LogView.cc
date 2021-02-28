#include "LogView.h"

#include <glibmm.h>

#include "colors.h"
#include "log.h"

LogLine::LogLine(const char *text, int height, Gdk::RGBA bg, Gdk::RGBA fg):
		Glib::ObjectBase("LogLine"), Gtk::Widget(),
		text_(text), height_(height), bg_(bg), fg_(fg) {
	set_has_window(true);
	set_name("log-line");

	Pango::FontDescription font;
	font.set_family("Monospace");
	font.set_size((height / 2) * Pango::SCALE);
	layout_ = create_pango_layout(text_);
	layout_->set_font_description(font);
}

void LogLine::setHighlighted(bool hl) {
	if (hl != isHighlighted_) {
		isHighlighted_ = hl;
		queue_draw();
	}
}

void LogLine::get_preferred_width_vfunc(int &min, int &nat) const {
	min = nat = layout_->get_logical_extents().get_width() / Pango::SCALE + HPADDING * 2;
}

void LogLine::get_preferred_width_for_height_vfunc(int h, int &min, int &nat) const {
	min = nat = layout_->get_logical_extents().get_width() / Pango::SCALE + HPADDING * 2;
}

void LogLine::get_preferred_height_vfunc(int &min, int &nat) const {
	min = nat = height_;
}

void LogLine::get_preferred_height_for_width_vfunc(int w, int &min, int &nat) const {
	min = nat = height_;
}

void LogLine::on_size_allocate(Gtk::Allocation &allocation) {
	set_allocation(allocation);
	if (get_window()) {
		get_window()->move_resize(
				allocation.get_x(), allocation.get_y(),
				allocation.get_width(), allocation.get_height());
	}
}

void LogLine::on_realize() {
	set_realized();

	if (!get_window()) {
		GdkWindowAttr attributes{};
		Gtk::Allocation allocation = get_allocation();
		attributes.x = allocation.get_x();
		attributes.y = allocation.get_y();
		attributes.width = allocation.get_width();
		attributes.height = allocation.get_height();
		attributes.event_mask = get_events() | Gdk::EXPOSURE_MASK;
		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.wclass = GDK_INPUT_OUTPUT;

		auto window = Gdk::Window::create(
				get_parent_window(), &attributes, GDK_WA_X | GDK_WA_Y);
		set_window(window);

		window->set_user_data(gobj());
	}
}

bool LogLine::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	Gdk::RGBA bg = isHighlighted_ ? highlightBgColor : bg_;
	Gdk::RGBA fg = isHighlighted_ ? textFgColor : fg_;

	const Gtk::Allocation alloc = get_allocation();
	cr->set_source_rgb(bg.get_red(), bg.get_green(), bg.get_blue());
	cr->move_to(0, 0);
	cr->line_to(alloc.get_width(), 0);
	cr->line_to(alloc.get_width(), alloc.get_height());
	cr->line_to(0, alloc.get_height());
	cr->fill();

	double textHeight = layout_->get_logical_extents().get_height() / (double)Pango::SCALE;
	double offset = (height_ - textHeight) / 2.0;

	cr->set_source_rgb(fg.get_red(), fg.get_green(), fg.get_blue());
	cr->move_to(HPADDING, offset);
	layout_->show_in_cairo_context(cr);

	return true;
}

LogView::LogView(Gdk::RGBA bg, Gdk::RGBA fg): bg_(bg), fg_(fg) {
	window_.add(container_);
	window_.get_vadjustment()->signal_value_changed().connect(
			sigc::mem_fun(this, &LogView::onScroll));
	window_.signal_size_allocate().connect(
			sigc::mem_fun(this, &LogView::onResize));

	searchWindow_.add(searchContainer_);

	paned_.pack1(window_, true, false);
	paned_.set_wide_handle(true);
}

void LogView::load(Glib::RefPtr<Gio::InputStream> stream) {
	reset();

	auto &lc = loadContext_;

	if (lc.cancelLoad.get()) {
		lc.cancelLoad->cancel();
	}

	lc.cancelLoad = Gio::Cancellable::create();

	lc.index = 0;
	lc.startIndex = 0;

	input_.resize(lc.index + lc.BUFSIZE);
	stream->read_async(
		input_.data(), lc.BUFSIZE, sigc::mem_fun(this, &LogView::onLoadData),
		lc.cancelLoad);
}

void LogView::setColors(Gdk::RGBA bg, Gdk::RGBA fg) {
	bg_ = bg;
	fg_ = fg;
	widgets_.clear();
	update();
}

void LogView::setPatterns(std::vector<std::shared_ptr<Pattern>> patterns) {
	patterns_ = std::move(patterns);
	widgets_.clear();
	update();
}

void LogView::patternsUpdated() {
	widgets_.clear();
	update();
}

void LogView::search(std::shared_ptr<Pattern> pattern) {
	unsearch();

	size_t count = 0;
	int width = maxWidth_;
	for (size_t i = 0; i < inputLines_.size(); ++i) {
		const char *line = input_.data() + inputLines_[i];
		if (!pattern->matches(line)) {
			continue;
		}

		if (count >= 1000) {
			searchResults_.push_back(SearchResult{
					{"[Over 1000 results, ignoring...]", pixelsPerLine_, bg_, fg_},
					i + 1});
			searchContainer_.put(searchResults_.back().widget, 0, count * pixelsPerLine_);
			break;
		}

		searchResults_.push_back(SearchResult{
				{line, pixelsPerLine_, pattern->bg_, pattern->fg_},
				i + 1});
		auto &result = searchResults_.back();

		searchContainer_.put(result.widget, 0, count * pixelsPerLine_);
		result.widget.show();

		std::string tooltip = "Line " + std::to_string(result.lineNum);
		result.widget.set_tooltip_text(tooltip);

		int min, nat;
		result.widget.get_preferred_width(min, nat);
		if (nat > width) {
			width = nat;
		}

		count += 1;
	}

	for (size_t i = 0; i < searchResults_.size(); ++i) {
		auto &result = searchResults_[i];
		result.widget.set_size_request(width);
		result.widget.set_events(result.widget.get_events() | Gdk::BUTTON_PRESS_MASK);
		result.widget.signal_button_press_event().connect([this, i](const GdkEventButton *evt) {
			if (evt->button == 1 && evt->type == GDK_BUTTON_PRESS) {
				auto &result = searchResults_[i];
				size_t dest = std::max((ssize_t)result.lineNum - 3, (ssize_t)0);
				window_.get_vadjustment()->set_value(dest * pixelsPerLine_);

				if (highlightedLine_ >= 0) {
					auto it = widgets_.find(highlightedLine_);
					if (it != widgets_.end()) {
						it->second->setHighlighted(false);
					}
				}

				highlightedLine_ = result.lineNum - 1;
				auto it = widgets_.find(highlightedLine_);
				if (it != widgets_.end()) {
					it->second->setHighlighted(true);
				}

				if (highlightedSearchResult_ >= 0) {
					searchResults_[highlightedSearchResult_].widget.setHighlighted(false);
				}

				highlightedSearchResult_ = i;
				result.widget.setHighlighted(true);

				return true;
			}
			return false;
		});
	}

	paned_.pack2(searchWindow_, false, true);
	searchWindow_.set_size_request(-1, std::min(count * pixelsPerLine_, (size_t)200));
	searchWindow_.show_all();

	searchPattern_ = std::move(pattern);
	widgets_.clear();
	update();
}

void LogView::unsearch() {
	searchPattern_.reset();
	searchResults_.clear();
	highlightedSearchResult_ = -1;
	highlightedLine_ = -1;
	widgets_.clear();
	paned_.remove(searchWindow_);
	update();
}

void LogView::reset() {
	unsearch();
	widgets_.clear();
	input_.clear();
	input_.shrink_to_fit();
	inputLines_.clear();
	inputLines_.shrink_to_fit();
	maxWidth_ = 0;
	update();
}

void LogView::update() {
	if (inputLines_.size() == 0) {
		return;
	}

	auto adjustment = window_.get_vadjustment();
	size_t baseLine = (ssize_t)(adjustment->get_value() / pixelsPerLine_);
	size_t firstLine = std::max(
			(ssize_t)baseLine - 5,
			(ssize_t)0);
	size_t lastLine = std::min(
			(ssize_t)(baseLine + (window_.get_height() / pixelsPerLine_) + 30),
			(ssize_t)inputLines_.size() - 1);

	// Remove invisible widgets
	std::vector<size_t> deleteList;
	for (auto &[key, widget]: widgets_) {
		if (key < firstLine || key > lastLine) {
			container_.remove(*widget);
			deleteList.push_back(key);
		}
	}

	// Need two loops, because deleting invalidates iterators
	for (size_t key: deleteList) {
		widgets_.erase(key);
	}

	// Load visible widgets
	int width = 0;
	for (size_t line = firstLine; line <= lastLine; ++line) {
		if (widgets_.find(line) != widgets_.end()) {
			continue;
		}

		// This could totally be sped up by keeping a widget cache,
		// but this actually seems more than fast enough
		auto widget = makeWidget(line);
		container_.put(*widget, 0, line * pixelsPerLine_);
		widget->show();

		int min, nat;
		widget->get_preferred_width(min, nat);
		if (nat > width) {
			width = nat;
		}

		widgets_[line] = std::move(widget);
	}

	// If we need to change the max width, just blow away everything and re-draw
	// with the new max width. Not the fastest in the world, but this happens rarely.
	if (width > maxWidth_) {
		maxWidth_ = width;
		container_.set_size_request(-1, inputLines_.size() * pixelsPerLine_);

		for (auto &[key, widget]: widgets_) {
			widget->set_size_request(maxWidth_, pixelsPerLine_);
		}
	}

	// This is kind of ugly, but meh
	if (maxWidth_ < window_.get_width()) {
		int w = window_.get_width();

		for (auto &[key, widget]: widgets_) {
			widget->set_size_request(w, pixelsPerLine_);
		}

		for (auto &result: searchResults_) {
			result.widget.set_size_request(w, pixelsPerLine_);
		}
	}
}

std::unique_ptr<LogLine> LogView::makeWidget(size_t line) {
	const char *text = input_.data() + inputLines_[line];
	Gdk::RGBA bg = bg_, fg = fg_;
	if (searchPattern_ && searchPattern_->matches(text)) {
		bg = searchPattern_->bg_;
		fg = searchPattern_->fg_;
	} else {
		for (auto &pattern: patterns_) {
			if (pattern->matches(text)) {
				bg = pattern->bg_;
				fg = pattern->fg_;
				break;
			}
		}
	}

	auto widget = std::make_unique<LogLine>(
			text, pixelsPerLine_, bg, fg);
	if (highlightedLine_ >= 0 && line == (size_t)highlightedLine_) {
		widget->setHighlighted(true);
	}
	widget->set_size_request(maxWidth_, pixelsPerLine_);
	return widget;
}

void LogView::onScroll() {
	update();
}

void LogView::onResize(Gdk::Rectangle &rect) {
	update();
}

void LogView::onLoadData(const Glib::RefPtr<Gio::AsyncResult> &result) {
	auto &lc = loadContext_;

	auto stream = Glib::RefPtr<Gio::InputStream>::cast_static(result->get_source_object());
	gssize n;
	try {
		n = stream->read_finish(result);
	} catch (const Glib::Error &err) {
		if (err.code() == Gio::Error::CANCELLED) {
			return; // Not exceptional, the stream just got cancelled
		}

		// Re-throw other errors though
		throw err;
	}

	if (n <= 0) {
		input_[lc.index++] = '\0';
		if (input_[lc.startIndex] != '\0') {
			inputLines_.push_back(lc.startIndex);
			update();
		}
		lc.startIndex = lc.index;
		return;
	}

	size_t end = lc.index + n;
	while (lc.index < end) {
		char &ch = input_[lc.index++];
		if (ch == '\n' || ch == '\r') {
			if (ch == '\r') {
				lc.index += 1;
			}
			ch = '\0';

			inputLines_.push_back(lc.startIndex);
			lc.startIndex = lc.index;

			container_.set_size_request(-1, inputLines_.size() * pixelsPerLine_);
			update();
		}
	}

	input_.resize(lc.index + lc.BUFSIZE);
	stream->read_async(
			input_.data() + lc.index, lc.BUFSIZE, sigc::mem_fun(this, &LogView::onLoadData),
			lc.cancelLoad);
}
