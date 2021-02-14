#include "LogView.h"

#include <glibmm.h>

#include "log.h"

LogLine::LogLine(const char *text, int width, int height, Gdk::RGBA bg, Gdk::RGBA fg):
		Glib::ObjectBase("LogLine"), Gtk::Widget(),
		text_(text), width_(width), height_(height), bg_(bg), fg_(fg) {
	set_has_window(true);
	set_name("log-line");

	Pango::FontDescription font;
	font.set_family("Monospace");
	layout_ = create_pango_layout(text_);
	layout_->set_font_description(font);
}

void LogLine::get_preferred_width_vfunc(int &min, int &nat) const {
	min = nat = std::max(layout_->get_logical_extents().get_width() / Pango::SCALE, width_);
}

void LogLine::get_preferred_width_for_height_vfunc(int h, int &min, int &nat) const {
	min = nat = std::max(layout_->get_logical_extents().get_width() / Pango::SCALE, width_);
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
	const Gtk::Allocation alloc = get_allocation();
	cr->set_source_rgb(bg_.get_red(), bg_.get_green(), bg_.get_blue());
	cr->move_to(0, 0);
	cr->line_to(alloc.get_width(), 0);
	cr->line_to(alloc.get_width(), alloc.get_height());
	cr->line_to(0, alloc.get_height());
	cr->fill();

	cr->set_source_rgb(fg_.get_red(), fg_.get_green(), fg_.get_blue());
	cr->move_to(0, 0);
	layout_->show_in_cairo_context(cr);

	return true;
}

LogView::LogView(Gdk::RGBA bg, Gdk::RGBA fg): bg_(bg), fg_(fg) {
	window_.add(container_);
	window_.get_vadjustment()->signal_value_changed().connect(
			sigc::mem_fun(this, &LogView::onScroll));
	window_.signal_size_allocate().connect(
			sigc::mem_fun(this, &LogView::onResize));
}

void LogView::load(Gio::InputStream &is) {
	gint64 startTime = g_get_monotonic_time();

	reset();
	size_t index = 0;
	const size_t bufsize = 4096;

	std::vector<size_t> indexes;
	indexes.push_back(0);

	while (1) {
		input_.resize(index + bufsize);
		ssize_t n = is.read(input_.data() + index, bufsize);
		if (n <= 0) {
			input_[index++] = '\0';
			break;
		}

		size_t end = index + n;
		while (index < end) {
			char &ch = input_[index++];
			if (ch == '\n' || ch == '\r') {
				ch = '\0';
				if (ch == '\r') {
					index += 1;
				}

				indexes.push_back(index);
			}
		}
	}

	// Newlines are often a terminator, not a separator
	if (input_[indexes.back()] == '\0') {
		indexes.pop_back();
	}

	gint64 duration = g_get_monotonic_time() - startTime;
	logln(
			"Loaded " << indexes.back() << " lines "
			<< "(" << input_.size() / 1000000.0 << "mb) in "
			<< (duration / 1000) << "ms.");

	inputLines_.reserve(indexes.size());
	for (size_t index: indexes) {
		inputLines_.push_back(input_.data() + index);
	}

	if (inputLines_.size() == 0) {
		update();
		return;
	}

	update();
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

void LogView::reset() {
	widgets_.clear();
	input_.clear();
	input_.shrink_to_fit();
	inputLines_.clear();
	inputLines_.shrink_to_fit();
	maxWidth_ = 0;
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
			(ssize_t)(baseLine + (window_.get_height() / pixelsPerLine_) + 5),
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
	for (size_t line = firstLine; line <= lastLine; ++line) {
		if (widgets_.find(line) != widgets_.end()) {
			continue;
		}

		// This could totally be sped up by keeping a widget cache,
		// but this actually seems more than fast enough
		auto widget = makeWidget(line);
		container_.put(*widget, 0, line * pixelsPerLine_);
		widget->show();
		widgets_[line] = std::move(widget);
	}

	// If we need to change the max width, just blow away everything and re-draw
	// with the new max width. Not the fastest in the world, but this happens rarely.
	if (container_.get_width() > maxWidth_) {
		maxWidth_ = container_.get_width();
		widgets_.clear();
		container_.set_size_request(-1, inputLines_.size() * pixelsPerLine_);
		update();
	}
}

std::unique_ptr<LogLine> LogView::makeWidget(size_t line) {
	const char *text = inputLines_[line];
	Gdk::RGBA bg = bg_, fg = fg_;
	for (auto &pattern: patterns_) {
		if (pattern->matches(text)) {
			bg = pattern->bg_;
			fg = pattern->fg_;
			break;
		}
	}

	auto widget = std::make_unique<LogLine>(
			text, maxWidth_, pixelsPerLine_, bg, fg);
	widget->set_size_request(maxWidth_, pixelsPerLine_);
	return widget;
}

void LogView::onScroll() {
	update();
}

void LogView::onResize(Gdk::Rectangle &rect) {
	update();
}
