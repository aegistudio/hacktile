// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file main.cpp
 * @author aegistudio
 * @brief Entrypoint for the hacktile CLI mode.
 *
 * This file is the entrypoint for hacktile-cli, which will
 * retrieve and setup the environment under command line
 * environment.
 *
 * TODO: current implementation is only supported on linux,
 * and we will support other platforms with libuv.
 */
#include "model/tetromino.hpp"
#include "model/tile.hpp"
#include "model/generator.hpp"
#include "model/playground.hpp"
#include "view/cli/terminal.hpp"
#include "view/cli/tile.hpp"
#include <signal.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <algorithm>
#include <sys/timerfd.h>
using namespace hacktile::model;
using namespace hacktile::view;

class mainPlaygroundView : public playgroundListener {
	cli::fullTileRenderer& current;
	cli::fullTileRenderer& shadow;
	cli::miniTileRenderer& preview;
	cli::terminal& term;
	playground& play;

	void repaintOutline();
	void repaintRangedField(uint8_t low, uint8_t high);
	void repaintTileField(const tile&, tileState);
	void repaintNewField(const tile&, tileState, tileState);
	void repaintField() {
		repaintRangedField(0, 20);
	}
	void repaintSwap();
	void repaintPreview();
public:
	mainPlaygroundView(
		cli::fullTileRenderer& current,
		cli::fullTileRenderer& shadow,
		cli::miniTileRenderer& preview,
		cli::terminal& term, playground& play):
		current(current), shadow(shadow), preview(preview),
		term(term), play(play) {
		repaintOutline();
		repaintField();
		repaintSwap();
		repaintPreview();
	}

	void tileSpawn(const tileSpawnEvent& event) {
		repaintPreview();
		repaintSwap();
		repaintNewField(event.type,
			event.location, event.locationShadow);
	}

	void tileSwap(const tileSwapEvent& event) {
		// TODO: provide more information in swap event.
		repaintField();
		repaintPreview();
		repaintSwap();
	}

	void tileLock(const tileLockEvent&) {
		repaintField();
		repaintPreview();
		repaintSwap();
	}

	void tileMove(const tileMoveEvent& event) {
		repaintTileField(event.type, event.before);
		repaintTileField(event.type, event.beforeShadow);
		repaintNewField(event.type,
			event.after, event.afterShadow);
	}
};

void mainPlaygroundView::repaintOutline() {
	// Render the outline, containing the outbox,
	// and section titles. The outline is the
	// lowest layer and all other elements should
	// be rendered over it.

	// Render the heading line.
	term << cli::pos(25, 5) << 
		cli::style::reset <<  "\xe2\x94\x8c"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x90";

	// Render the internal lines.
	for(int i = 6; i <= 25; ++ i) {
		term << cli::pos(25, i) <<
			cli::style::reset <<
			"\xe2\x94\x82"
			"                    "
			"\xe2\x94\x82";
	}

	// Render the trailing line.
	term << cli::pos(25, 25) << 
		cli::style::reset <<  "\xe2\x94\x94"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x80" "\xe2\x94\x80"
		"\xe2\x94\x98";

	// Render the section banners.
	term << cli::foreground(cli::color::black)
		<< cli::background(cli::color::green)
		<< cli::pos(47, 6)  << " // SWAP     "
		<< cli::pos(47, 10) << " // PREVIEW  "
		<< cli::pos(12, 6)  << "     // GOAL "
		<< cli::pos(12, 12) << "    // STATS "
		<< cli::style::reset;
}

void mainPlaygroundView::repaintSwap() {
	// Always clear the panel of the swap section.
	term << cli::style::reset
		<< cli::pos(50, 7) << "     "
		<< cli::pos(50, 8) << "     "
		<< cli::pos(50, 9) << "     ";

	// Render the swap tiles on the right.
	const tile* swap = play.getSwapTile();
	if(swap != nullptr) {
		term << cli::pos(50, 10);
		if(play.isSwapEnabled()) {
			preview.renderTile(term, *swap);
		} else {
			term << cli::foreground(cli::color::white);
			preview.renderTile(term, *swap,
				enumTileDirection::initial, false);
		}
	}
}

void mainPlaygroundView::repaintPreview() {
	// Render the preview tiles on the right.
	for(int i = 0; i < 5 && i < play.getNumPreviews(); ++ i) {
		term << cli::style::reset
			<< cli::pos(50, 11+3*i) << "     "
			<< cli::pos(50, 12+3*i) << "     "
			<< cli::pos(50, 13+3*i) << "     ";
		const tile* current = play.getPreview(i);
		if(current != nullptr) {
			term << cli::pos(50, 14+3*i);
			preview.renderTile(term, *current);
		}
	}
}

void mainPlaygroundView::repaintRangedField(
	uint8_t low, uint8_t high) {
	for(uint8_t i = low; i <= high; ++i)
		term << cli::pos(26, 24-i) << "                    ";
	term << cli::pos(26, 24);
	current.renderField(term, play.getField(), low, high);
}

void mainPlaygroundView::repaintTileField(
	const tile& type, tileState state) {
	tileCoord leftBottom, rightTop;
	type.retrieveBoundingBox(state.dir, leftBottom, rightTop);
	repaintRangedField(state.y + leftBottom.y, state.y + rightTop.y);
}

void mainPlaygroundView::repaintNewField(
	const tile& type, tileState state, tileState stateShadow) {
	repaintTileField(type, state);
	repaintTileField(type, stateShadow);
	// TODO: move the relocating algorithm to another view.
	term << cli::pos(26+2*stateShadow.x, 24-stateShadow.y);
	shadow.renderTile(term, type, stateShadow.dir, true);
	term << cli::pos(26+2*state.x, 24-state.y);
	current.renderTile(term, type, state.dir, true);
}

int main(int argc, char** argv) {
	// Initialize the terminal object for displaying.
	cli::terminal term(1);

	// Initialize the tiles, in the order of the enum.
	std::vector<tile> tiles;
	for(uint8_t i = 1; i <= 7; ++ i) {
		tileData data;
		createTetrominoTileData(data, tetromino(i));
		tileRotationTable kick;
		createTetrominoRotation(kick, tetromino(i));
		tiles.emplace_back(tile(data, kick));
	}
	std::vector<const tile*> tilePointers;
	for(uint8_t i = 1; i <= 7; ++ i) {
		tilePointers.emplace_back(&tiles[i-1]);
	}

	// Initialize the tiles' palette, but viewed from the
	// data in tile, not type of tile itself.
	uint8_t paletteColor[8];
	paletteColor[0] = 0;
	paletteColor[int(tetromino::J)] = cli::color::yellow;
	paletteColor[int(tetromino::L)] = cli::color::blue;
	paletteColor[int(tetromino::S)] = cli::color::green;
	paletteColor[int(tetromino::Z)] = cli::color::red;
	paletteColor[int(tetromino::T)] = cli::color::magenta;
	paletteColor[int(tetromino::I)] = cli::color::cyan;
	paletteColor[int(tetromino::O)] = cli::color::bright|cli::color::yellow;
	const char* paletteCurrent[8];
	const char* paletteShadow[8];
	paletteCurrent[0] = nullptr;
	paletteShadow[0] = nullptr;
	for(uint8_t i = 1; i <= 7; ++ i) {
		paletteCurrent[i] = "\xe2\x96\x88\xe2\x96\x88";
		paletteShadow[i] = "\xe2\x96\x92\xe2\x96\x92";
	}
	cli::fullTileRenderer current(paletteColor, paletteCurrent, 8);
	cli::fullTileRenderer shadow(paletteColor, paletteShadow, 8);
	cli::miniTileRenderer preview(paletteColor, 8);

	// Initialize the game playground model for game.
	tilePermutator permutator(tilePointers.data(), 7, 0);
	playground play(&permutator);

	// Initialize the playground view of the game.
	mainPlaygroundView playView(current, shadow, preview, term, play);
	auto subscription = play.subscribe(&playView);

	// Execute the main loop of the game.
	play.start();
	while(1) {
		// Flush out the content from terminal.
		term.flush();

		// Initialize the poll descriptor, including the
		// user input and the timer.
		pollfd fds[1];
		fds[0].fd = 1;
		fds[0].events = POLLIN;
		fds[0].revents = 0;

		// Poll for more events in the loop.
		if(poll(fds, 1, -1) < 0) return -errno;

		// Attempt to accept input from the input.
		if((fds[0].revents & POLLIN) != 0) {
			char c[2048];
			ssize_t len = read(1, c, sizeof(c));
			if(len < 0) return -errno;
			for(ssize_t i = 0; i < len; i ++) {
				char k = c[i];
				if(k == '\3') return 0; // Ctrl+C
				switch(k) {
				case 'q': play.swapTile(); break;
				case 'd': play.rotateCW(); break;
				case 'a': play.rotateCCW(); break;
				case 'w': play.halfTurn(); break;
				case '4': play.move(-1); break;
				case '6': play.move(+1); break;
				case '7': play.move(-10); break;
				case '9': play.move(+10); break;
				case '5': play.drop(+20); break;
				case '8': play.drop(+1); break;
				case 's': play.hardDrop(); break;
				}
			}
		}
	}
	return 0;
}
