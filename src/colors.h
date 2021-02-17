#pragma once

#include <array>
#include <gdkmm/rgba.h>

inline Gdk::RGBA operator""_rgba(const char *str, size_t len) {
	Gdk::RGBA rgba{str};
	rgba.set_alpha(1);
	return rgba;
}

inline const Gdk::RGBA textBgColor = "#ffffff"_rgba;
inline const Gdk::RGBA textFgColor = "#000000"_rgba;
inline const Gdk::RGBA searchBgColor = "#24eeff"_rgba;
inline const Gdk::RGBA highlightBgColor = "#4aaaff"_rgba;

inline const Gdk::RGBA patternBgColors[] = {
	"#ff9c85"_rgba,
	"#b5be00"_rgba,
	"#00ce9e"_rgba,
	"#00c6f4"_rgba,
	"#e09cff"_rgba,
	"#ff98ae"_rgba,
	"#d8b300"_rgba,
	"#00d25f"_rgba,
	"#00c9d6"_rgba,
	"#b7acff"_rgba,
	"#ff94cc"_rgba,
	"#fba300"_rgba,
	"#82c900"_rgba,
	"#00ccbd"_rgba,
	"#81b9ff"_rgba,
	"#ff8df0"_rgba,
};
inline const size_t patternBgColorsLen = sizeof(patternBgColors) / sizeof(*patternBgColors);
