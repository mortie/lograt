#include "Pattern.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

Pattern::Pattern(std::string rx, Gdk::RGBA bg, Gdk::RGBA fg) noexcept:
		rx_(std::move(rx)), bg_(bg), fg_(fg) {}

Pattern::Pattern(Pattern &&pat) noexcept:
		rx_(std::move(pat.rx_)), bg_(pat.bg_), fg_(pat.fg_),
		code_(pat.code_), md_(pat.md_) {
	pat.code_ = nullptr;
	pat.md_ = nullptr;
}

Pattern::~Pattern() {
	if (code_) {
		pcre2_code_free((pcre2_code *)code_);
		pcre2_match_data_free((pcre2_match_data *)md_);
	}
}

Pattern &Pattern::operator=(Pattern &&pat) {
	rx_ = std::move(pat.rx_);
	bg_ = pat.bg_;
	fg_ = pat.fg_;
	code_ = pat.code_;
	md_ = pat.md_;

	pat.code_ = nullptr;
	pat.md_ = nullptr;
	return *this;
}

std::string Pattern::compile() {
	int errnum;
	size_t erroffset;
	code_ = pcre2_compile(
			(PCRE2_SPTR)rx_.c_str(), PCRE2_ZERO_TERMINATED, 0,
			&errnum, &erroffset, NULL);
	if (code_ == nullptr) {
		char buffer[512];
		pcre2_get_error_message(errnum, (PCRE2_UCHAR *)buffer, sizeof(buffer));
		return std::string("Offset ") + std::to_string(erroffset) + ": " + buffer;
	}

	md_ = pcre2_match_data_create_from_pattern((pcre2_code *)code_, nullptr);
	return "";
}

bool Pattern::matches(const char *str) {
	if (!isOk()) {
		return false;
	}

	int ret = pcre2_match(
			(pcre2_code *)code_, (PCRE2_SPTR)str, PCRE2_ZERO_TERMINATED, 0, 0,
			(pcre2_match_data *)md_, nullptr);
	return ret >= 0;
}
