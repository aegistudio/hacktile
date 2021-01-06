#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file tetromino.hpp
 * @brief tetromino specific definition
 * @author aegistudio
 *
 * This file provides definitions dedicated for tetromino,
 * including setup of tile data, and rotation system.
 */
#include "model/tile.hpp"

namespace hacktile {
namespace model {

/**
 * @brief tetromino is the enumeration of all possible tiles
 * in the tetris, while providing their names.
 */
enum class tetromino : uint8_t {
	J = 1,
	L,
	S,
	Z,
	T,
	I,
	O,
};

/**
 * createTetrominoTileData constructs and returns the tile
 * data corresponding to the tile with specified value.
 */
void createTetrominoTileData(
	tileData result, tetromino typ, uint8_t value = 0);

/**
 * createTetrominoRotation constructs and returns the
 * recommended rotation table of the tile.
 */
void createTetrominoRotation(
	tileRotationTable result, tetromino typ);

} // namespace hacktile::model
} // namespace hacktile
