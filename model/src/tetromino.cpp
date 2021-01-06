// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file tetromino.cpp
 * @author aegistudio
 * @brief Implementation of some concrete logic related
 * to tetromino.
 *
 * This file implements the logic of tetromino, including the
 * initialization of certain tile data, commonly used wall
 * kick table, and so on.
 */
#include "model/tetromino.hpp"
#include <stdexcept>
#include <cstring>

namespace hacktile {
namespace model {

void createTetrominoTileData(
	tileData result, tetromino typ, uint8_t value) {

	// Initialize the varaibles for tile data preparation.
	if(value == 0) value = uint8_t(typ);

	// Bitmap for representing the data of tile.
	constexpr uint8_t _ = 0; uint8_t $ = value;
	switch(typ) {
	case tetromino::J: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, _, _, _, _ },
				{ _, $, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			}, {
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			}, {
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, $, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, _, _, _ },
			}, {
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::L: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, $, _, _ },
				{ _, $, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, $, _, _ },
				{ _, $, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::S: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, _, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::Z: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, $, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::I: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, $, $, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, $, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, $, $, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::O: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	case tetromino::T: {
		tileData data = {
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, $, $, $, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, $, $, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, $, $, $, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
			{
				{ _, _, _, _, _, _ },
				{ _, _, _, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, $, $, _, _, _ },
				{ _, _, $, _, _, _ },
				{ _, _, _, _, _, _ },
			},
		};
		memcpy(result, data, sizeof(data));
	}; break;
	default:
		throw std::runtime_error("invalid tetromino type");
	}
}

void createTetrominoRotation(
	tileRotationTable result, tetromino typ) {

	// Initialize all operations with 0 value.
	for(int i = 0; i < 4; ++ i)
		for(int j = 0; j < 4; ++ j)
			result[i][j][0] = 0;

	// Initialize the monotonic right rotation first.
	switch(typ) {
	case tetromino::J:
	case tetromino::L:
	case tetromino::S:
	case tetromino::Z:
	case tetromino::T: {
		// Default rotation recommended for 3x3 bounded tile.

		// tileDirection::initial -> tileDirection::right
		result[0][1][0] = tileCoordAt(-1,  0);
		result[0][1][1] = tileCoordAt(-1, +1);
		result[0][1][2] = tileCoordAt( 0, -2);
		result[0][1][3] = tileCoordAt(-1, -2);
		result[0][1][4] = 0;

		// tileDirection::right -> tileDirection::halfTurned
		result[1][2][0] = tileCoordAt(+1,  0);
		result[1][2][1] = tileCoordAt(+1, -1);
		result[1][2][2] = tileCoordAt( 0, +2);
		result[1][2][3] = tileCoordAt(+1, +2);
		result[1][2][4] = 0;

		// tileDirection::halfTurned -> tileDirection::left
		result[2][3][0] = tileCoordAt(+1,  0);
		result[2][3][1] = tileCoordAt(+1, +1);
		result[2][3][2] = tileCoordAt( 0, -2);
		result[2][3][3] = tileCoordAt(+1, -2);
		result[2][3][4] = 0;

		// tileDirection::left -> tileDirection::initial
		result[3][0][0] = tileCoordAt(-1,  0);
		result[3][0][1] = tileCoordAt(-1, -1);
		result[3][0][2] = tileCoordAt( 0, +2);
		result[3][0][3] = tileCoordAt(-1, +2);
		result[3][0][4] = 0;
	}; break;
	case tetromino::I: {
		// Default rotation for tetromino::I.
		
		// tileDirection::initial -> tileDirection::right
		result[0][1][0] = tileCoordAt(-2,  0);
		result[0][1][1] = tileCoordAt(+1,  0);
		result[0][1][2] = tileCoordAt(-2, -1);
		result[0][1][3] = tileCoordAt(+1, +2);
		result[0][1][4] = 0;

		// tileDirection::right -> tileDirection::halfTurned
		result[1][2][1] = tileCoordAt(-1,  0);
		result[1][2][0] = tileCoordAt(+2,  0);
		result[1][2][2] = tileCoordAt(-1, +2);
		result[1][2][3] = tileCoordAt(+2, -1);
		result[1][2][4] = 0;

		// tileDirection::halfTurned -> tileDirection::left
		result[2][3][0] = tileCoordAt(+2,  0);
		result[2][3][1] = tileCoordAt(-1,  0);
		result[2][3][2] = tileCoordAt(+2, +1);
		result[2][3][3] = tileCoordAt(-1, -2);
		result[2][3][4] = 0;

		// tileDirection::left -> tileDirection::initial
		result[3][0][1] = tileCoordAt(+1,  0);
		result[3][0][0] = tileCoordAt(-2,  0);
		result[3][0][2] = tileCoordAt(+1, -2);
		result[3][0][3] = tileCoordAt(-2, +1);
		result[3][0][4] = 0;
	}; break;
	case tetromino::O: {
		// No rotation for the tetromino::O.
	}; break;
	default:
		throw std::runtime_error("invalid tetromino type");
	}

	// Inverse transform for the reversed operations.
	for(int i = 0; i < 4; ++ i)
		for(int j = 0; j < 4; ++ j)
			for(int k = 0; k < 5; ++ k) {
				tileCoord coord;
				coord.value = result[i][j][k];
				if(coord.value == 0) break;
				coord.x = -coord.x;
				coord.y = -coord.y;
				result[j][i][k] = coord.value;
			}
}

} // namespace hacktile::model
} // namespace hacktile
