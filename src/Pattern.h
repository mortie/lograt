#pragma once

#include <gdkmm.h>
#include <string>

class Pattern {
public:
	Pattern(std::string rx, Gdk::RGBA bg, Gdk::RGBA fg) noexcept;
	Pattern(Pattern &&pat) noexcept;
	Pattern(const Pattern &) = delete;
	~Pattern();

	Pattern &operator=(Pattern &&pat);
	Pattern &operator=(const Pattern &) = delete;

	std::string compile();
	bool matches(const char *str);
	bool isOk() { return code_ != nullptr; }

	std::string rx_;
	Gdk::RGBA bg_;
	Gdk::RGBA fg_;

private:
	void *code_ = nullptr;
	void *md_ = nullptr;
};
