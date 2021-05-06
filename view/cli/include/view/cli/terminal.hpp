#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file terminal.hpp
 * @brief terminal interfaces definition.
 * @author aegistudio
 *
 * This file provides interface definition for terminal
 * relative objects. This provides a primitive and raw
 * interface for manipulating next output location and
 * output color.
 */
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include <termios.h>

namespace hacktile {
namespace view {
namespace cli {

/**
 * @brief color specifies the native form of console
 * coloring natively.
 */
namespace color {
	enum {
		// name = 0b0BGR
		black   = 0b0000,
		red     = 0b0001,
		green   = 0b0010,
		yellow  = 0b0011,
		blue    = 0b0100,
		magenta = 0b0101,
		cyan    = 0b0110,
		white   = 0b0111,
		bright  = 0b1000,
	};
} // namespace hacktile::view::cli::color

/**
 * @brief decoration are available decorations used
 * while rendering text to terminal.
 */
enum class style : uint8_t {
	reset       = 0x0,
	highlight   = 0x1,
	underscore  = 0x4,
	blink       = 0x5,
	invert      = 0x7,
	erase       = 0x8,
};

/**
 * @brief background is an output type for the terminal
 * to update barely the background.
 */
struct background {
	uint8_t value;
	constexpr background(uint8_t value): value(value) {}
};

/**
 * @brief foreground is an output type of the terminal
 * to update barely the foreground.
 */
struct foreground {
	uint8_t value;
	constexpr foreground(uint8_t value): value(value) {}
};

/**
 * @brief move represents a single text movement, relative
 * to the current cursor location.
 */
struct move {
	int x, y;
	constexpr move(int x, int y): x(x), y(y) {}
};

/**
 * @brief pos represents a single text movement, absolute
 * in the screen location.
 */
struct pos {
	int x, y;
	constexpr pos(int x, int y): x(x), y(y) {}
};

/**
 * @brief terminal provides interface for other instance
 * classes to manipulate the terminal.
 *
 * This class initializes the screen subsystem, change the
 * current output style and location, and finally synchronize
 * the update to the player screen.
 */
class terminal {
	struct term_initializer final {
		termios terminalMode;
		int term;
		term_initializer(int term);
		~term_initializer();
	} initialized_term;
	struct new_screen_setter final {
		new_screen_setter(term_initializer &t);
	} new_screen;
	struct screen_cleaner final {
		int term;
		screen_cleaner(int term);
		~screen_cleaner();
	} cleaned_screen;
	int term;
	std::vector<char> buffer;
	uint8_t foregroundColor, backgroundColor;
	style currentStyle;
	bool styleUpdated, hasBackground;
	void appendStyleSequence();
public:
	terminal(int term);
	terminal& operator<<(foreground);
	terminal& operator<<(background);
	terminal& operator<<(move);
	terminal& operator<<(pos);
	terminal& operator<<(style);
	void append(const char*, size_t);

	// Delegated method of appending a string.
	terminal& operator<<(const std::string& s) {
		append(s.c_str(), s.length());
		return *this;
	}

	// Delegated method of appending a general C-string.
	terminal& operator<<(const char* s) {
		append(s, ::strlen(s));
		return *this;
	}
	
	// Delegated method of appending a fix-sized string.
	template<size_t length>
	terminal& operator<<(const char s[length]) {
		append(s, length);
		return *this;
	}

	void flush();
};

} // namespace hacktile::view::cli
} // namespace hacktile::view
} // namespace hacktile
