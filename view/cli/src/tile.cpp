// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file tile.cpp
 * @author aegistudio
 * @brief Implementation of the tile renderer interface.
 *
 * This file provides tile render interfaces based on
 * the terminal interface. And as terminal interface might
 * be refactored in the future, the implementtion will also
 * be changed in the future.
 */
#include "view/cli/tile.hpp"
using namespace hacktile::model;

namespace hacktile {
namespace view {
namespace cli {

terminal& fullTileRenderer::renderTile(terminal& output,
	const tile& which, tileDirection dir, bool renderColor) {

	uint8_t rdata[tile::maxNumPixels];
	tileCoord rloc[tile::maxNumPixels];
	int numPixels = which.retrieveTileData(dir, rdata, rloc);
	if(renderColor) output << style::reset;
	tileCoord previous;
	previous.value = 0;
	for(int i = 0; i < numPixels; ++ i) {
		if(rloc[i].y != previous.y) {
			output << move(- previous.x * 2,
				previous.y - rloc[i].y);
			previous.x = 0; previous.y = rloc[i].y;
		}
		if(rloc[i].x != previous.x) {
			output << move(
				(rloc[i].x - previous.x) * 2, 0);
			previous.x = rloc[i].x;
		}
		if(renderColor) output << foreground(col[rdata[i]]);
		output << character[rdata[i]];
		previous.x ++;
	}
	return output;
}

terminal& fullTileRenderer::renderField(terminal& output,
	const field& f, uint8_t bottom, uint8_t top) {

	if(bottom > 0) output << move(0, -bottom);
	for(uint8_t r = bottom; r <= top; ++ r) {
		fieldRow row = f.rowAt(r);
		for(int i = 0; i < 10; ++ i) {
			if(row[i] == 0 || row[i] >= length) {
				output << move(2, 0);
				continue;
			}
			output << foreground(col[row[i]]);
			output << character[row[i]];
		}
		output << move(-20, -1);
	}
	return output;
}

terminal& miniTileRenderer::renderTile(terminal& output,
	const tile& which, tileDirection dir, bool renderColor) {

	uint8_t rdata[tile::maxNumPixels];
	tileCoord rloc[tile::maxNumPixels];
	uint8_t data[6][6] = {0};
	int numPixels = which.retrieveTileData(dir, rdata, rloc);
	for(int i = 0; i < numPixels; ++ i) {
		data[rloc[i].y][rloc[i].x] = rdata[i];
	}
	tileCoord leftBottom, rightTop;
	which.retrieveBoundingBox(dir, leftBottom, rightTop);
	uint8_t clamped = leftBottom.y & ~uint8_t(1);
	if(renderColor) output << style::reset;
	output << move(leftBottom.x, -clamped);
	for(uint8_t y = clamped; y <= rightTop.y; y += 2) {
		for(uint8_t x = leftBottom.x; x <= rightTop.x; ++ x) {
			if(data[y][x] == data[y+1][x]) {
				if(data[y][x] == 0)
					output << move(1, 0);
				else {
					if(renderColor)
						output << foreground(col[data[y][x]]);
					output << "\xe2\x96\x88";
				}
			} else if(data[y+1][x] == 0) {
				if(renderColor)
					output << foreground(col[data[y][x]]);
				output << "\xe2\x96\x84";
			} else if(data[y][x] == 0) {
				if(renderColor)
					output << foreground(col[data[y+1][x]]);
				output << "\xe2\x96\x80";
			} else {
				if(renderColor)
					output << foreground(col[data[y][x]])
						<< background(col[data[y+1][x]]);
				output << "\xe2\x96\x84";
			}
		}
		output << move(-(rightTop.x - leftBottom.x), -1);
	}
	return output;
}

} // namespace hacktile::view::cli
} // namespace hacktile::view
} // namespace hacktile
