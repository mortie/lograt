#pragma once

#include <string>

struct Color { float r, g, b; };

class Pattern {
public:
	Pattern(std::string rx, Color bg, Color fg);
	~Pattern();

	std::string compile();
	bool matches(const char *str);

	const std::string rx_;
	const Color bg_;
	const Color fg_;

private:
	void *code_ = nullptr;
	void *md_ = nullptr;
};
