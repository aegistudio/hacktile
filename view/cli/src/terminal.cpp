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
#include "view/cli/terminal.hpp"
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <stdexcept>

namespace hacktile {
namespace view {
namespace cli {

#define control "\033["

terminal::term_initializer::term_initializer(int term):
	term(term) {
	// Retrieve the original flag of terminal, so that they
	// could be recovered once the program has exit.
	if(tcgetattr(term, &terminalMode) < 0) {
		std::stringstream error;
		error << "cannot fetch terminal attribute: "
			<< strerror(errno);
		throw std::runtime_error(error.str());
	}
}
terminal::term_initializer::~term_initializer() {
	// Reset the screen display mode.
	tcsetattr(term, TCSANOW, &terminalMode);
}
terminal::new_screen_setter::new_screen_setter(term_initializer &t) {
	// Attempt to copy and modify the console mode.
	termios newTerminalMode;
	memcpy(&newTerminalMode, &t.terminalMode, sizeof(termios));
	newTerminalMode.c_iflag &= ~(
		IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	newTerminalMode.c_oflag &= ~(OCRNL|XTABS);
	newTerminalMode.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	newTerminalMode.c_cc[VMIN] = 1;
	newTerminalMode.c_cc[VTIME] = 0;
	if(tcsetattr(t.term, TCSANOW, &newTerminalMode) < 0) {
		std::stringstream error;
		error << "cannot initialize terminal: "
			<< strerror(errno);
		throw std::runtime_error(error.str());
	}
}

terminal::screen_cleaner::screen_cleaner(int term): term(term) {
	// Initialize the current screen for printing.
	char initScreen[] = control "2J" control "0;0H" control "?25l";
	size_t lenInitScreen = sizeof(initScreen);
	if(write(term, initScreen, lenInitScreen) != lenInitScreen) {
		std::stringstream error;
		error << "cannot setup terminal screen: "
			<< strerror(errno);
		throw std::runtime_error(error.str());
	}
}
terminal::screen_cleaner::~screen_cleaner() {
	// Clear screen data and reset the pointer.
	char resetPointer[] = control "0;0H" control "?25h" control "2J";
	write(term, resetPointer, sizeof(resetPointer));
}

terminal::terminal(int term):
	initialized_term(term), new_screen(initialized_term), cleaned_screen(term),
	term(term), foregroundColor(color::white), backgroundColor(color::black),
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
	if(write(term, buffer.data(), buffer.size()) != buffer.size()) {
		std::stringstream error;
		error << "cannot write to terminal: " << strerror(errno);
		throw std::runtime_error(error.str());
	} else {
		buffer = std::vector<char>();
	}
}

} // namespace hacktile::view::cli
} // namespace hacktile::view
} // namespace hacktile
