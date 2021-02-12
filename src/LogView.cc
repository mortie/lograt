#include "LogView.h"

#include <glibmm.h>

#include "log.h"

LogView::LogView() {
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
				indexes.push_back(index);
			}
		}
	}

	// Newlines are often a terminator, not a separator
	if (input_[indexes.back()] == '\0') {
		indexes.pop_back();
	}

	gint64 duration = g_get_monotonic_time() - startTime;
	logln("Loaded " << indexes.back() << " lines in " << (duration / 1000) << "ms.");

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

void LogView::reset() {
	widgets_.clear();
	input_.clear();
	input_.shrink_to_fit();
	inputLines_.clear();
	inputLines_.shrink_to_fit();
}

void LogView::update() {
	if (inputLines_.size() == 0) {
		return;
	}

	auto adjustment = window_.get_vadjustment();
	size_t firstLine = std::max(
			(ssize_t)(adjustment->get_value() / pixelsPerLine_) - 5,
			(ssize_t)0);
	size_t lastLine = std::min(
			(ssize_t)(firstLine + (window_.get_height() / pixelsPerLine_) + 5),
			(ssize_t)inputLines_.size() - 1);

	// Remove invisible widgets
	std::vector<size_t> deleteList;
	for (auto &[key, widget]: widgets_) {
		if (key < firstLine || key > lastLine) {
			container_.remove(*widget);
			deleteList.push_back(key);
		}
	}

	// Need twoo loops, because deleting invalidates iterators
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
		auto label = std::make_unique<Gtk::Label>(inputLines_[line]);
		container_.put(*label, 0, line * pixelsPerLine_);
		label->show();
		widgets_[line] = std::move(label);
	}

	if (container_.get_width() > maxWidth_) {
		maxWidth_ = container_.get_width();
		container_.set_size_request(maxWidth_, inputLines_.size() * pixelsPerLine_);
	}
}

void LogView::onScroll() {
	update();
}

void LogView::onResize(Gdk::Rectangle &rect) {
	update();
}
