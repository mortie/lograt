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
inline const Gdk::RGBA highlightBgColor = "#83bcff"_rgba;

inline const Gdk::RGBA patternBgColors[] = {
	"#f9f06b"_rgba,
	"#ffbe6f"_rgba,
	"#f66151"_rgba,
	"#dc8add"_rgba,
	"#cdaB8f"_rgba,
	"#99c1f1"_rgba,
	"#8ff0a4"_rgba,
};
inline const size_t patternBgColorsLen = sizeof(patternBgColors) / sizeof(*patternBgColors);
