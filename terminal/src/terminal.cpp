// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file terminal.cpp
 * @brief Implementation of the terminal interface.
 * @author aegistudio
 *
 * This file implements the pseudo terminal interface, which
 * should initialize the terminal environment and provides
 * interface for managing content in termimnal.
 *
 * Currently a relatively naive interface is implemented,
 * which will provides a writer interface to flush the
 * current content out every loop, and the caller is able
 * to write out control characters on content. We might
 * switch to libuv in the future.
 */
#include "util/defer.hpp"
#include "terminal/terminal.hpp"
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <string>

namespace {
struct charSpan {
	const char *content;
	std::size_t len;
	charSpan(const char *content, const std::size_t len):
		content(content), len(len) {}
	template <std::size_t N>
	charSpan(const char (&content)[N]): charSpan(content, N) {}
};
void onError(int return_code, const charSpan error_prefix) {
	if (return_code < 0) {
		std::string error_message{error_prefix.content, error_prefix.len};
		error_message += strerror(errno);
		throw std::runtime_error(std::move(error_message));
	}
}
void write(int term, const charSpan content, const charSpan error_prefix) {
	onError(
		::write(term, content.content, content.len) != content.len ? -1 : 0,
		error_prefix
	);
}
}

namespace hacktile {
namespace terminal {

#define control "\033["

namespace details {
initializedTerminal::initializedTerminal(int term):
	term(term) {
	// Retrieve the original flag of terminal, so that they
	// could be recovered once the program has exit.
	onError(tcgetattr(term, &terminalMode),
		"cannot fetch terminal attribute: ");
}
initializedTerminal::~initializedTerminal() {
	// Reset the screen display mode.
	tcsetattr(term, TCSANOW, &terminalMode);
}

newTerminal::newTerminal(int term): initializedTerminal(term) {
	// Attempt to copy and modify the console mode.
	termios newTerminalMode;
	memcpy(&newTerminalMode, &terminalMode, sizeof(termios));
	newTerminalMode.c_iflag &= ~(
		IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	newTerminalMode.c_oflag &= ~(OCRNL|XTABS);
	newTerminalMode.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	newTerminalMode.c_cc[VMIN] = 1;
	newTerminalMode.c_cc[VTIME] = 0;
	onError(tcsetattr(term, TCSANOW, &newTerminalMode),
		"cannot initialize terminal: ");
}

clearScreen::clearScreen(int term): newTerminal(term) {
	// Initialize the current screen for printing.
	write(term, control "2J" control "0;0H" control "?25l",
		"cannot setup terminal screen: ");
}
clearScreen::~clearScreen() {
	// Clear screen data and reset the pointer.
	write(term, control "0;0H" control "?25h" control "2J",
		"cannot reset pointer: ");
}
}

terminal::terminal(int term): clearScreen(term),
	foregroundColor(color::white), backgroundColor(color::black),
	currentStyle(style::reset), styleUpdated(true), hasBackground(false) {}

void terminal::appendStyleSequence() {
	char buf[15];
	buf[0] = '\033';
	buf[1] = '[';
	buf[2] = '0' + uint8_t(currentStyle);
	buf[3] = ';';
	buf[4] = (foregroundColor&color::bright)? '9' : '3';
	buf[5] = '0' + (foregroundColor & 0b0111);

	// Terminate the case when there's no background.
	if(!hasBackground) {
		buf[6] = 'm';
		buffer.insert(buffer.end(), &buf[0], &buf[7]);
		return;
	}
	buf[6] = ';';

	// Depending on whether the background is dark, we
	// will append different sequence.
	if(backgroundColor&color::bright) {
		buf[7] = '1';
		buf[8] = '0';
		buf[9] = '0' + (backgroundColor & 0b0111);
		buf[10] = 'm';
		buffer.insert(buffer.end(), &buf[0], &buf[11]);
	} else {
		buf[7] = '4';
		buf[8] = '0' + (backgroundColor & 0b0111);
		buf[9] = 'm';
		buffer.insert(buffer.end(), &buf[0], &buf[10]);
	}
}

terminal& terminal::operator<<(foreground fg) {
	if(foregroundColor != fg.value) {
		foregroundColor = fg.value;
		styleUpdated = true;
	}
	return *this;
}

terminal& terminal::operator<<(background bg) {
	if(!hasBackground || backgroundColor != bg.value) {
		backgroundColor = bg.value;
		hasBackground = true;
		styleUpdated = true;
	}
	return *this;
}

terminal& terminal::operator<<(style s) {
	if(s == style::reset) {
		if(hasBackground || foregroundColor != color::white) {
			*this << "\033[0m"; // Simplify the reset precedure.
			hasBackground = false;
			backgroundColor = color::black;
			foregroundColor = color::white;
			styleUpdated = false;
		}
	}
	if(currentStyle != s) {
		currentStyle = s;
		styleUpdated = true;
	}
	return *this;
}

terminal& terminal::operator<<(move m) {
	if(m.x != 0) {
		char buf[256];
		buf[0] = '\033';
		buf[1] = '[';
		int len = snprintf(&buf[2], sizeof(buf)-2, "%d%c",
			m.x < 0? -m.x:m.x, m.x < 0? 'D':'C');
		buffer.insert(buffer.end(), &buf[0], &buf[len+2]);
	}
	if(m.y != 0) {
		char buf[256];
		buf[0] = '\033';
		buf[1] = '[';
		int len = snprintf(&buf[2], sizeof(buf)-2, "%d%c",
			m.y < 0? -m.y:m.y, m.y < 0? 'A':'B');
		buffer.insert(buffer.end(), &buf[0], &buf[len+2]);
	}
	return *this;
}

terminal& terminal::operator<<(pos p) {
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	char buf[256];
	buf[0] = '\033';
	buf[1] = '[';
	int len = snprintf(&buf[2], sizeof(buf)-2,
		"%d;%dH", p.y, p.x);
	buffer.insert(buffer.end(), &buf[0], &buf[len+2]);
	return *this;
}

void terminal::append(const char* s, size_t length) {
	if(styleUpdated) {
		appendStyleSequence();
		styleUpdated = false;
	}
	buffer.insert(buffer.end(), &s[0], &s[length]);
}

void terminal::flush() {
	write(term, charSpan{buffer.data(), buffer.size()},
		"cannot write to terminal: ");
	buffer = std::vector<char>();
}

} // namespace hacktile::terminal
} // namespace hacktile
