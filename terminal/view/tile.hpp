#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file tile.hpp
 * @brief tile renderer interfaces definition.
 * @author aegistudio
 *
 * This file provides interface definition for rendering
 * tile at specified location. A full tile renderer for
 * rendering tile in the field and a semi tile renderer
 * for rendering tiles in half will also be provided.
 */
#include "terminal/terminal.hpp"
#include "model/tile.hpp"

namespace hacktile {
namespace terminal {
namespace view {

/**
 * @brief fullTileRenderer is for rendering a full
 * tile within specified 6x12 column region.
 *
 * The renderer will select color and character from
 * palette while rendering. Data specified at palette
 * 0 will always be ignored since it provides no data.
 *
 * Each character specified must takes up to one row and
 * two columns otherwise the rendering will garble.
 */
class fullTileRenderer {
	uint8_t* col;
	const char** character;
	size_t length;
public:
	constexpr fullTileRenderer(
		uint8_t* col, const char** character, size_t length):
		col(col), character(character), length(length) {
	}

	/// renderTile will attempt to render a tile right at
	/// specified location.
	terminal& renderTile(
		hacktile::terminal::terminal& output,
		const hacktile::model::tile& which,
		hacktile::model::tileDirection dir =
			hacktile::model::enumTileDirection::initial,
		bool renderColor = true);

	/// renderField will attempt to render a field starting
	/// at cursor location in the unit of row ranges.
	terminal& renderField(
		hacktile::terminal::terminal& output,
		const hacktile::model::field& field,
		uint8_t bottom = 0, uint8_t top = 20);
}; // class hacktile::view::cli::fullTileRenderer.

/**
 * @brief miniTileRenderer is for rendering a mini
 * tile within specified 3row*6column region.
 *
 * The renderer will select color from palette however
 * all tiles will be rendererd as block. This is for
 * the case when tile should be shown in preview or
 * other players field in room more than 2 players.
 */
class miniTileRenderer {
	uint8_t* col;
	size_t length;
public:
	constexpr miniTileRenderer(uint8_t* col, size_t length):
		col(col), length(length) {}

	/// renderTile will attempt to render a tile right at
	/// specified location.
	terminal& renderTile(
		hacktile::terminal::terminal& output,
		const hacktile::model::tile& which,
		hacktile::model::tileDirection dir =
			hacktile::model::enumTileDirection::initial,
		bool renderColor = true);
}; // class hacktile::view::cli::miniTileRenderer.

} // namespace hacktile::view::cli
} // namespace hacktile::view
} // namespace hacktile
