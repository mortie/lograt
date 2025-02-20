#include "LogView.h"

#include <glibmm.h>

#include "colors.h"
#include "log.h"

LogLine::LogLine(const char *text, int height, Gdk::RGBA bg, Gdk::RGBA fg):
		height_(height), bg_(bg), fg_(fg) {
	Pango::FontDescription font;
	font.set_family("Monospace");
	font.set_size((height / 2) * Pango::SCALE);

	layout_ = area_.create_pango_layout(text);
	layout_->set_font_description(font);

	area_.set_content_width(
		layout_->get_logical_extents().get_width() / Pango::SCALE + HPADDING * 2);
	area_.set_content_height(
		layout_->get_logical_extents().get_height() / Pango::SCALE + HPADDING * 2);
	area_.set_draw_func(sigc::mem_fun(*this, &LogLine::draw));
}

void LogLine::setHighlighted(bool hl) {
	if (hl != isHighlighted_) {
		isHighlighted_ = hl;
		area_.queue_draw();

		if (hl) {
			Pango::AttrList attrs;
			auto bold = Pango::Attribute::create_attr_weight(Pango::Weight::BOLD);
			attrs.insert(bold);
			layout_->set_attributes(attrs);
		} else {
			Pango::AttrList attrs;
			layout_->set_attributes(attrs);
		}
	}
}

void LogLine::draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {
	Gdk::RGBA bg = isHighlighted_ ? fg_ : bg_;
	Gdk::RGBA fg = isHighlighted_ ? bg_ : fg_;

	cr->set_source_rgb(bg.get_red(), bg.get_green(), bg.get_blue());
	cr->move_to(0, 0);
	cr->line_to(width, 0);
	cr->line_to(width, height);
	cr->line_to(0, height);
	cr->fill();

	double textHeight = layout_->get_logical_extents().get_height() / (double)Pango::SCALE;
	double offset = (height_ - textHeight) / 2.0;

	cr->set_source_rgb(fg.get_red(), fg.get_green(), fg.get_blue());
	cr->move_to(HPADDING, offset);
	layout_->show_in_cairo_context(cr);
}

LogView::LogView(Gdk::RGBA bg, Gdk::RGBA fg): bg_(bg), fg_(fg) {
	window_.set_child(container_);
	window_.get_vadjustment()->signal_value_changed().connect(
			sigc::mem_fun(*this, &LogView::onScroll));

	searchWindow_.set_child(searchContainer_);

	paned_.set_start_child(window_);
	paned_.set_resize_start_child(true);
	paned_.set_shrink_start_child(false);
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
		input_.data(), lc.BUFSIZE, sigc::mem_fun(*this, &LogView::onLoadData),
		lc.cancelLoad);
}

void LogView::setPatterns(std::vector<std::shared_ptr<Pattern>> patterns) {
	patterns_ = std::move(patterns);
	lines_.clear();
	update();
}

void LogView::patternsUpdated() {
	lines_.clear();
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
			searchContainer_.put(searchResults_.back().line.widget(), 0, count * pixelsPerLine_);
			break;
		}

		searchResults_.push_back(SearchResult{
				{line, pixelsPerLine_, pattern->bg_, pattern->fg_},
				i + 1});
		auto &result = searchResults_.back();

		searchContainer_.put(result.line.widget(), 0, count * pixelsPerLine_);
		result.line.widget().show();

		std::string tooltip = "Line " + std::to_string(result.lineNum);
		result.line.widget().set_tooltip_text(tooltip);

		Gtk::Requisition min, nat;
		result.line.widget().get_preferred_size(min, nat);
		if (nat.get_width() > width) {
			width = nat.get_width();
		}

		count += 1;
	}

	for (size_t i = 0; i < searchResults_.size(); ++i) {
		auto &result = searchResults_[i];
		result.line.widget().set_size_request(width);

		auto clickGesture = Gtk::GestureClick::create();
		clickGesture->signal_pressed().connect([this, i](int n, double x, double y) {
			auto &result = searchResults_[i];
			size_t dest = std::max((ssize_t)result.lineNum - 3, (ssize_t)0);
			window_.get_vadjustment()->set_value(dest * pixelsPerLine_);

			if (highlightedLine_ >= 0) {
				auto it = lines_.find(highlightedLine_);
				if (it != lines_.end()) {
					it->second->setHighlighted(false);
				}
			}

			highlightedLine_ = result.lineNum - 1;
			auto it = lines_.find(highlightedLine_);
			if (it != lines_.end()) {
				it->second->setHighlighted(true);
			}

			if (highlightedSearchResult_ >= 0) {
				searchResults_[highlightedSearchResult_].line.setHighlighted(false);
			}

			highlightedSearchResult_ = i;
			result.line.setHighlighted(true);
		});

		result.line.widget().add_controller(std::move(clickGesture));
	}

	paned_.set_end_child(searchWindow_);
	paned_.set_resize_end_child(false);
	paned_.set_shrink_end_child(true);
	searchWindow_.set_size_request(-1, std::min(count * pixelsPerLine_, (size_t)200));

	searchPattern_ = std::move(pattern);
	lines_.clear();
	update();
}

void LogView::unsearch() {
	searchPattern_.reset();
	searchResults_.clear();
	highlightedSearchResult_ = -1;
	highlightedLine_ = -1;
	lines_.clear();
	paned_.unset_end_child();
	update();
}

void LogView::reset() {
	unsearch();
	lines_.clear();
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
	for (auto &[key, line]: lines_) {
		if (key < firstLine || key > lastLine) {
			container_.remove(line->widget());
			deleteList.push_back(key);
		}
	}

	// Need two loops, because deleting invalidates iterators
	for (size_t key: deleteList) {
		lines_.erase(key);
	}

	// Load visible widgets
	int width = 0;
	for (size_t l = firstLine; l <= lastLine; ++l) {
		if (lines_.find(l) != lines_.end()) {
			continue;
		}

		// This could totally be sped up by keeping a widget cache,
		// but this actually seems more than fast enough
		auto line = makeLine(l);
		container_.put(line->widget(), 0, l * pixelsPerLine_);
		line->widget().show();

		Gtk::Requisition min, nat;
		line->widget().get_preferred_size(min, nat);
		if (nat.get_width() > width) {
			width = nat.get_width();
		}

		lines_[l] = std::move(line);
	}

	// If we need to change the max width, just blow away everything and re-draw
	// with the new max width. Not the fastest in the world, but this happens rarely.
	if (width > maxWidth_) {
		maxWidth_ = width;
		container_.set_size_request(-1, inputLines_.size() * pixelsPerLine_);

		for (auto &[key, line]: lines_) {
			line->widget().set_size_request(maxWidth_, pixelsPerLine_);
		}
	}

	// This is kind of ugly, but meh
	if (maxWidth_ < window_.get_width()) {
		int w = window_.get_width();

		for (auto &[key, line]: lines_) {
			line->widget().set_size_request(w, pixelsPerLine_);
		}

		for (auto &result: searchResults_) {
			result.line.widget().set_size_request(w, pixelsPerLine_);
		}
	}
}

std::unique_ptr<LogLine> LogView::makeLine(size_t num) {
	const char *text = input_.data() + inputLines_[num];
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

	auto line = std::make_unique<LogLine>(
			text, pixelsPerLine_, bg, fg);
	if (highlightedLine_ >= 0 && num == (size_t)highlightedLine_) {
		line->setHighlighted(true);
	}
	line->widget().set_size_request(maxWidth_, pixelsPerLine_);
	return line;
}

void LogView::onScroll() {
	update();
}

void LogView::onLoadData(const Glib::RefPtr<Gio::AsyncResult> &result) {
	auto &lc = loadContext_;

	auto stream = std::dynamic_pointer_cast<Gio::InputStream>(result->get_source_object_base());
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

	bool foundNewLine = false;
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

			foundNewLine = true;
		}
	}

	if (foundNewLine) {
		container_.set_size_request(-1, inputLines_.size() * pixelsPerLine_);
		update();

		// TODO: Don't do this horribly inefficiently
		if (searchPattern_) {
			search(std::move(searchPattern_));
		}
	}

	input_.resize(lc.index + lc.BUFSIZE);
	stream->read_async(
			input_.data() + lc.index, lc.BUFSIZE, sigc::mem_fun(*this, &LogView::onLoadData),
			lc.cancelLoad);
}
