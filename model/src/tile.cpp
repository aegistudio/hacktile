// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file tile.cpp
 * @brief Implementation of some concrete logic related to tile.
 *
 * This file implements some concrete logic of tile. Normally,
 * the hacktile application must link with this file, and the
 * modules might link to this file too when they requires some
 * logic associated with field.
 *
 * The latter case will cause each module to include an exact
 * copy of the code of that class, but it is expected since we
 * don't want the methods defined in this file to appear in
 * the PLT/GOT table, which slows down the algorithm.
 */
#include "model/tile.hpp"
#include <stdexcept>
#include <type_traits>

namespace hacktile {
namespace model {

tile::tile(tile::dataType input, tile::rotationType rotation) {
	// Initialize coordinate data of the tile.
	for(uint8_t i = 0; i < 4; ++ i) {
		uint8_t numPixels = 0;
		tileCoord vmin; vmin.value = 0;
		tileCoord vmax; vmax.value = 0;
		compactTile[i] = 0;
		for(uint8_t y = 0; y < 6; ++ y) {
			for(uint8_t x = 0; x < 6; ++ x) {
				uint8_t v = input[i][5-y][x];
				if(v == 0) continue;
				if(numPixels >= maxNumPixels)
					throw std::runtime_error("too many pixels");

				data[i][numPixels] = v;
				loc[i][numPixels] = tileCoordAt(x, y);
				compactTile[i] |= (uint64_t(1)<<x) << (10*y);
				if(vmin.x == 0 || vmin.x > x) vmin.x = x;
				if(vmin.y == 0 || vmin.y > y) vmin.y = y;
				if(vmax.x == 0 || vmax.x < x) vmax.x = x;
				if(vmax.y == 0 || vmax.y < y) vmax.y = y;
				++ numPixels;
			}
		}
		if(numPixels < maxNumPixels) {
			data[i][numPixels] = 0;
			loc[i][numPixels] = 0;
		}
		min[i] = vmin.value;
		max[i] = vmax.value;
	}

	// Initialize rotation data of the tile.
	for(uint8_t i = 0; i < 4; ++ i) {
		for(uint8_t j = 0; j < 4; ++ j) {
			uint8_t numRotations = 0;
			while(rotation[i][j][numRotations] != 0) {
				if(numRotations >= maxNumRotations)
					throw std::runtime_error("too many rotations");
				rotateTable[i][j][numRotations]
					= rotation[i][j][numRotations];
				++ numRotations;
			}
			if(numRotations < maxNumRotations)
				rotateTable[i][j][numRotations] = 0;
		}
	}
}

tileState tile::initTileState() const {
	tileState state;
	state.dir = enumTileDirection::initial;
	state.x = 2; // Must always be 2 when inside 6x6 square.
	tileCoord coord; coord.value = min[0];
	state.y = 19 - coord.y; // lowest line must be in row 19.
	return state;
}

void field::assertLegit(const tilePathFinder& pfd) const {
	if(pfd.version != 0 && pfd.version != version)
		throw std::runtime_error("mismatched field version");
}

bool field::isValid(const tilePathFinder& pfd) {
	tileState state = pfd.getState();
	uint8_t dir = state.dir.getValue();
	const auto& typ = *pfd.typ;

	// Bounding box checking ensures the tile will
	// remain inside the boundary in the operation.
	tileCoord min; min.value = typ.min[dir];
	tileCoord max; max.value = typ.max[dir];
	if(state.x + min.x <   0) return false;
	if(state.x + max.x >= 10) return false;
	if(state.y + max.y <   0) return false;

	// Check whether the specified item has collided
	// with the user tile.
	compactField compactTile(typ.compactTile[dir]);
	compactTile = compactTile.tileMove(state.x);
	return !pfd.current.collide(compactTile);
}

void field::envalidate(tilePathFinder& pfd) const {
	pfd.version = version;
}

bool field::spawn(tilePathFinder& pfd) const {
	if(pfd.typ == nullptr)
		throw std::runtime_error("tile not specified");
	tileState state = pfd.state;
	pfd.current = compactField();
	for(int i = 6; i > 0; -- i) pfd.current =
		pfd.current.fieldDown(
			compactRowAt(state.y + i - 1));
	if(!isValid(pfd)) return false;
	envalidate(pfd);
	return true;
}

bool field::move(const tilePathFinder& pfd,
	int8_t numSteps, tilePathFinder& result) const {
	assertLegit(pfd);
	result = tilePathFinder();

	// Prepare argument for field calculation.
	compactField current = pfd.current;
	tileState state = pfd.state;
	uint8_t dir = state.dir.getValue();
	const auto& typ = *pfd.typ;
	compactField tile = typ.compactTile[dir];
	int8_t x = state.x;
	tile = tile.tileMove(x);

	// Move left or right based on the offset.
	if(numSteps > 0) {
		tileCoord max; max.value = typ.max[dir];
		while(x + max.x < 9 && numSteps > 0) {
			++ x;
			tile = tile.tileMove(1);
			if(current.collide(tile)) {
				-- x;
				break;
			}
			-- numSteps;
		}
	} else if(numSteps < 0) {
		tileCoord min; min.value = typ.min[dir];
		 while(x + min.x > 0 && numSteps < 0) {
			-- x;
			tile = tile.tileMove(-1);
			if(current.collide(tile)) {
				++ x;
				break;
			}
			++ numSteps;
		}
	}

	// If the tile cannot be moved (or not moved),
	// we will return false directly.
	if(x == state.x) return false;

	// Move the tile to specified location and return.
	result = tilePathFinder(pfd.typ, state);
	result.current = pfd.current;
	result.state.x = x;
	envalidate(result);
	return true;
}

bool field::drop(const tilePathFinder& pfd,
	uint8_t numSteps, tilePathFinder& result) const {
	assertLegit(pfd);
	result = tilePathFinder();

	// TODO: skip over empty lines so that we will only
	// need to consider lines with tile.

	// From now on, move one line at a time until there's
	// no more step to move or next step will collide with
	// the current line.
	tileState state = pfd.getState();
	uint8_t dir = state.dir.getValue();
	result = tilePathFinder(pfd.typ, state);
	result.current = pfd.current;
	const auto& typ = *pfd.typ;
	uint8_t inSteps = numSteps;
	compactField tile = typ.compactTile[dir];
	tile = tile.tileMove(state.x);
	tileCoord min; min.value = typ.min[dir];
	while(numSteps > 0) {
		compactField current = result.current.fieldDown(
			compactRowAt(result.state.y - 1));
		if(current.collide(tile)) break;
		-- result.state.y;
		result.current = current;
		-- numSteps;
	}

	// See whether we have moved down any step, and return
	// false if no step is moved.
	if(inSteps == numSteps) {
		result = tilePathFinder();
		return false;
	}

	// Otherwise return the moved result.
	envalidate(result);
	return true;
}

bool field::rotate(const tilePathFinder& pfd,
	tileDirection targetDir, tilePathFinder& result) const {
	assertLegit(pfd);
	tileState state = pfd.getState();

	// Initialize the state first, and attempt to apply
	// the rotation table.
	result = tilePathFinder(pfd.typ, state);
	result.state.dir = targetDir;
	result.current = pfd.current;

	// Judge whether the initial state is valid.
	if(isValid(result)) {
		envalidate(result);
		return true;
	}

	// We must apply the rotation table / wall kicking
	// table and see whether there's acceptible rotation.
	const auto& typ = *pfd.typ;
	uint8_t srcDir = state.dir.getValue();
	uint8_t dstDir = targetDir.getValue();
	const auto& table = typ.rotateTable[srcDir][dstDir];
	for(int n = 0; n < tile::maxNumRotations; ++ n) {
		uint8_t value = table[n];
		if(value == 0) break;
		tileCoord coord; coord.value = value;

		// Shift the result state to attempt to next
		// specified rotation state.
		result.state.x = state.x + coord.x;
		int8_t newStateY = state.y + coord.y;
		while(result.state.y < newStateY) {
			result.current = result.current.fieldUp(
				compactRowAt(result.state.y + 6));
			++ result.state.y;
		}
		while(result.state.y > newStateY) {
			result.current = result.current.fieldDown(
				compactRowAt(result.state.y - 1));
			-- result.state.y;
		}

		// Terminate the attempt once we've found a
		// valid rotation state.
		if(isValid(result)) {
			result.previousWallKick = true;
			envalidate(result);
			return true;
		}
	}

	// None of the rotation is applicable, so we would
	// return false to indicate no available rotation.
	result = tilePathFinder();
	return false;
}

bool field::lock(const tilePathFinder& pfd, uint8_t& clear) {
	assertLegit(pfd);
	clear = 0;
	const auto& typ = *pfd.typ;

	// Lock precondition: current tile location is valid,
	// and cannot drop further more.
	if(!isValid(pfd)) return false;
	tilePathFinder nextCondition;
	if(drop(pfd, 1, nextCondition)) return false;

	// Add more tiles based on the bounding box.
	tileState state = pfd.getState();
	uint8_t dir = state.dir.getValue();
	tileCoord min; min.value = typ.min[dir];
	tileCoord max; max.value = typ.max[dir];
	while(state.y + max.y >= fields.size()) {
		fields.push_back(fieldRow());
		compactFields.push_back(0);
	}
	for(uint8_t n = 0; n < tile::maxNumPixels; ++ n) {
		uint8_t data = typ.data[dir][n];
		if(data == 0) break;
		tileCoord loc; loc.value = typ.loc[dir][n];
		uint8_t x = state.x + loc.x;
		uint8_t y = state.y + loc.y;

		// Add data of current tile to the field.
		fields[y][x] = data;
		compactFields[y] |= (1<<x);
	}

	// Erase the rows that has already been occupied.
	for(uint8_t j = min.y; j <= max.y; ++ j) {
		uint8_t row = state.y + j - clear;
		if(compactFields[row] == solidRow) {
			fields.erase(fields.begin() + row);
			compactFields.erase(compactFields.begin() + row);
			++ clear;
		}
	}
	++ version;
	return true;
}

void field::grow(fieldRow row) {
	uint16_t compactLine = 0;
	for(int i = 0; i < 10; ++ i) 
		compactLine |= (row[i] != 0)? (1<<i) : 0;
	fields.insert(fields.begin(), std::move(row));
	compactFields.insert(compactFields.begin(), compactLine);
	++ version;
}

} // namespace hacktile::model
} // namespace hacktile
