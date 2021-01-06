// SPDX-License-Identifier: LGPL-3.0-or-later
#include <gtest/gtest.h>
#include "model/tile.hpp"
#include "model/tetromino.hpp"
using namespace hacktile::model;

// Tile.TSpinMini is the testing for dropping a tetromino::T
// onto a single line with leftmost pixel being hole.
// This should result in a T-spin Mini and erase single line.
TEST(Tile, TSpinMini) {
	// Initialize the tile data for T-block.
	tileData data;
	createTetrominoTileData(data, tetromino::T);
	tileRotationTable kick;
	createTetrominoRotation(kick, tetromino::T);
	tile t(data, kick);

	// Initialize the field data with single line with hole.
	field f;
	f.grow({0, 1, 1, 1, 1, 1, 1, 1, 1, 1});

	// Attempt to spawn the T-block in the field.
	tilePathFinder pfd(&t);
	ASSERT_TRUE(f.spawn(pfd));

	// Attempt to press soft drop and see whether the tile
	// lands at the row.
	{
		tilePathFinder npfd;
		ASSERT_TRUE(f.drop(pfd, 20, npfd));
		pfd = npfd;
		tileState state = pfd.getState();
		ASSERT_EQ(state.x, +2); // Still at the center.
		ASSERT_EQ(state.y, -1); // -1 + 2 = 1.
	}

	// And we attempt to drop again, this time, the
	// tile will not be moved.
	{
		tilePathFinder npfd;
		ASSERT_FALSE(f.drop(pfd, 20, npfd));
	}

	// Attempt to press move leftmost and see whether the
	// tile hits the left wall.
	{
		tilePathFinder npfd;
		ASSERT_TRUE(f.move(pfd, -10, npfd));
		pfd = npfd;
		tileState state = pfd.getState();
		ASSERT_EQ(state.x, -1); // -1 + 1 = 0.
		ASSERT_EQ(state.y, -1); // Still over the row.
	}

	// Attempt to press spin right and trigger a wall kick.
	{
		tilePathFinder npfd;
		ASSERT_TRUE(f.rotate(pfd,
			enumTileDirection::right, npfd));
		pfd = npfd;
		tileState state = pfd.getState();
		ASSERT_EQ(state.x, -2);
		ASSERT_EQ(state.y, -1);
	}

	// Attempt to lock the tile, and a single line erase
	// will be expected.
	uint8_t clear = 0;
	ASSERT_TRUE(f.lock(pfd, clear));
	ASSERT_EQ(clear, 1);
	ASSERT_EQ(f.compactRowAt(0), 3);
	ASSERT_EQ(f.compactRowAt(1), 1);
}
