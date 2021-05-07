// SPDF-License-Identifier: LGPL-3.0-or-later
#pragma once

/**
 * @file tile.hpp
 * @brief tile collision and rotation system algorithm
 * @author aegistudio
 *
 * This file provides a general *mino (tetromino,
 * pentamino, etc.) tile collision and rotation algorithm
 * support, with provided field storage.
 *
 * Each mino tile could store extra data of the tile so
 * that they could be used in the later evaluation. The
 * field will be constrained to 10 columns per row, and
 * you will need to rewrite a dedicated field algorithm if
 * your requirement is really specific.
 */
#include <cstdint>
#include <algorithm>
#include <vector>
#include <array>

namespace hacktile {
namespace model {

/**
 * @brief compactField is a special type of field that
 * supports fast collision detection by creating each
 * compact fields using bit operations.
 *
 * On architectures that does not support 64-bit integer
 * bit operations, this data structure should still
 * yield effective comparision efficiency, since it will
 * not require by-bit comparisions.
 *
 * By definition, each compact field contains a 6*10
 * field, and if the user would like to support different
 * field schemas, they should never use it.
 *
 * XXX: For the case that the rotated bounding box is
 * outside the field, we must apply the bounding box
 * algorithm prior to compact field comparison.
 */
class compactField {
	uint64_t field;

	/// fieldMask is the mask to select the lowest
	/// 10-bits from each row.
	static constexpr uint64_t fieldMask = (((uint64_t)1)<<10)-1;

	/// fullMask is the mask for selecting only the
	/// rows inside the field.
	static constexpr uint64_t fullMask = (((uint64_t)1)<<60)-1;
public:
	/// compactField creates a new instance of field.
	compactField(uint64_t data = 0): field(data) {}

	/// assignment operator for the field.
	compactField(const compactField& c): field(c.field) {}

	/// collide judges whether a field has collided with
	/// an input tile. The comparision is as simple as
	/// comparing their fields.
	bool collide(const compactField& f) const {
		return (field & f.field) != 0;
	}

	/// tileMove is used dedicated for tiles to move
	/// it horizontally.
	///
	/// Please notice that the operation will only be
	/// meaningful for tiles, and it could move out of
	/// boundaries, which requires extra check from
	/// its callers.
	compactField tileMove(int8_t x) const {
		uint64_t result = field;
		if(x >= 0) result <<= x;
		else result >>= (-x);
		return compactField(result);
	}

	/// fieldDown is used for adding a line to bottom
	/// and erase the top line.
	compactField fieldDown(uint16_t row) const {
		uint64_t result = field;
		result <<= 10;
		result &= fullMask;
		uint64_t rowData = row & fieldMask;
		result |= rowData;
		return compactField(result);
	}

	/// fieldUp is used for adding a line to the top
	/// and erase the bottom line.
	compactField fieldUp(uint64_t row) const {
		uint64_t result = field;
		result &= fullMask;
		result >>= 10;
		uint64_t rowData = row & fieldMask;
		result |= (rowData << 50);
		return compactField(result);
	}
}; // struct hacktile::model::compactField

/**
 * @brief a tile direction indicates the orientation of
 * a tile, while providing handy function for evaluating
 * rotations.
 */
class tileDirection {
	uint8_t value;
public:
	/// default constructor initializes the direction to
	/// the initial rotation.
	tileDirection(): value(0) {}

	/// constexpr constructor defined for enum defining.
	constexpr tileDirection(uint8_t value): value(value) {
		// XXX: just an intention for the code users.
		//static_assert(0 <= value && value < 3,
		//	"invalid value for tile direction");
	}

	/// rotateCW returns the value after clockwise rotation.
	tileDirection rotateCW() const {
		return tileDirection((value+1) & 0x03);
	}

	/// rotateCCW returns the value after counter-clockwise
	/// rotation.
	tileDirection rotateCCW() const  {
		return tileDirection((value-1) & 0x03);
	}

	/// halfTurn returns the value after rotate 180 degree.
	tileDirection halfTurn() const {
		return tileDirection((value+2) & 0x03);
	}

	/// equal operator of the tile direction.
	bool operator==(const tileDirection& dir) const {
		return value == dir.value;
	}

	/// getValue returns the tile direction.
	uint8_t getValue() const {
		return value;
	}
}; // struct hacktile::model::tileDirection

// enumTileDirection defines the constant expression values
// that could be used in code related to rotation.
namespace enumTileDirection {
	static constexpr tileDirection initial    = 0;
	static constexpr tileDirection right      = 1;
	static constexpr tileDirection halfTurned = 2;
	static constexpr tileDirection left       = 3;
} // namespace hacktile::model::enumTileDirection

/**
 * @brief a tile state stores the information of the
 * tile held by the user, it includes coordinate and
 * the state of the tile.
 */
struct tileState {
	tileDirection dir;
	int8_t x, y;

	tileState(): dir(), x(0), y(0) {}

	tileState(const tileState& r):
		dir(r.dir), x(r.x), y(r.y) {}
}; // struct hacktile::model::tileState

/**
 * @brief tileCoord used as coordinates in the system.
 */
union tileCoord {
	uint8_t value;
	struct {
		int8_t x : 4;
		int8_t y : 4;
	};
}; // hacktile::model::tileCoord

/**
 * @brief tileCoordAt returns the value associated to coord.
 */
inline uint8_t tileCoordAt(int8_t x, int8_t y) {
	tileCoord coord;
	coord.x = x;
	coord.y = y;
	return coord.value;
}

/**
 * @brief a tile is the currently passed data indicating
 * the current item in use and will be used for collision
 * detection, insertion and so on later.
 *
 * Size of each tile is 6x6, which holds most tetromino
 * and pentamino, and enable a compact bit operation with
 * the compact field to detect collision, and apply the
 * rotation system argument.
 */
struct tile {
	/// maxNumPixels is maximum pixel amount in a tile.
	static constexpr int maxNumPixels = 8;

	/// maxNumRotations is maximum rotation allowed in a tile.
	static constexpr int maxNumRotations = 12;

	/// dataType is the data defined dedicated for tile.
	typedef uint8_t dataType[4][6][6];

	/// rotationType is the rotation table defined for tile.
	typedef uint8_t rotationType[4][4][maxNumRotations];
private:
	uint8_t data[4][maxNumPixels];
	uint8_t loc[4][maxNumPixels];
	uint8_t min[4], max[4];
	uint64_t compactTile[4];
	rotationType rotateTable;
	friend class field;
public:
	// tile constructor to completely specify the basic
	// bounding boxes and rotation states.
	//
	// XXX: data argument is turned upside-down, which
	// means the arg[?][0][0] actually represents the
	// pixel at data[?][5][0], so are other tiles. This
	// is used to simplify most code and make them readable.
	tile(dataType, rotationType);

	// initTileState evaluates the location of where
	// should the tile spawn in field.
	tileState initTileState() const;

	// retrieveTileData is used for retrieving tile data
	// back from the tile object, the result will be copied
	// into the specified buffer and number of pixels will
	// also be returned.
	int retrieveTileData(tileDirection dir,
		uint8_t rdata[], tileCoord rloc[]) const {
		for(int i = 0; i < maxNumPixels; ++ i) {
			rdata[i] = data[dir.getValue()][i];
			rloc[i].value = loc[dir.getValue()][i];
			if(rloc[i].value == 0) return i;
		}
		return maxNumPixels;
	}

	// retrieveBoundingBox retrieves the bounding box of tile.
	void retrieveBoundingBox(tileDirection dir,
		tileCoord& rmin, tileCoord& rmax) const {
		rmin.value = min[dir.getValue()];
		rmax.value = max[dir.getValue()];
	}
}; // struct hacktile::model::tile

/// tileData is forwarded type for tile data.
typedef tile::dataType tileData;

/// tileRotationTable is forwarded type for rotation table.
typedef tile::rotationType tileRotationTable;

/**
 * @brief tilePathFinder is the path finder of a tile,
 * whose internal state could be modified by the
 * hacktile::field to ensure that it must be valid.
 */
class tilePathFinder {
	const tile* typ;
	tileState state;
	uint64_t version;

	// Used only for storing the previously calculated
	// result here.
	compactField current;

	/// only the class hacktile::field can access it.
	friend class field;

	/// previousWallKick is a field indicates that the
	/// previous step has been an wall kick.
	bool previousWallKick;
public:
	/// tilePathFinder null constructor.
	tilePathFinder():
		typ(nullptr), state(), version(0), current(0),
		previousWallKick(false) {}

	/// tilePathFinder default constructor creates a
	/// tile path finder with specified tile state.
	tilePathFinder(
		const tile* typ, const tileState& state):
		typ(typ), state(state), version(0), current(0),
		previousWallKick(false) {}

	/// tilePathFinder type-only constructor for
	/// evaluating the default location prior to any
	/// other operations.
	tilePathFinder(const tile* typ):
		tilePathFinder(typ, typ->initTileState()) {}

	/// assignment operator to reinitialize the fields.
	tilePathFinder& operator=(tilePathFinder pfd) {
		typ = pfd.typ;
		state = pfd.state;
		version = pfd.version;
		current = pfd.current;
		previousWallKick = pfd.previousWallKick;
		return *this;
	}

	/// getType returns the current piece type.
	const tile* getType() const {
		return typ;
	}

	/// getState retrieves the internal state of the
	/// tile path finder.
	const tileState& getState() const {
		return state;
	}

	/// isPreviousWallKick is just used to record whether
	/// this path finder is created when the previous
	/// operation is a wall kick.
	bool isPreviousWallKick() const {
		return previousWallKick;
	}
}; // struct hacktile::model::tilePathFinder

/**
 * @brief a field row definition comes in handy.
 */
typedef std::array<uint8_t, 10> fieldRow;

/**
 * @brief field is a concrete playground of a player.
 *
 * Each field has always 10 columns and must contains at
 * least 22 lines, and the caller could evaluate the result
 * of horizontal move, drop, bottomDrop, rotate and flip.
 */
class field {
	std::vector<uint16_t> compactFields;
	std::vector<fieldRow> fields;
	uint64_t version;

	/// assertLegit is a state that could be used in
	/// methods other than spawn.
	void assertLegit(const tilePathFinder&) const;

	/// isValid judges whether the current path finder
	/// generated could be used as the next state.
	static bool isValid(const tilePathFinder&);

	/// envalidate is used to mark the state valid.
	void envalidate(tilePathFinder&) const;
public:
	/// field initialize the current field, including the
	/// specification of the fields and compactFields.
	field(): compactFields(), fields(), version(1) {
		compactFields.reserve(22);
		fields.reserve(22);
	}

	/// spawn attempt to create a tile with initial rotation
	/// at initial location, and returns whether it could be
	/// spawned. This could be used as a termination condition.
	bool spawn(tilePathFinder&) const;

	/// solidRow is a special type of row indicates the
	/// block has filled the row.
	///
	/// Please notice that when the solid row is used as
	/// garbage line, it will not be erased by player, since
	/// only the tile placed by player will trigger line clear.
	static constexpr uint16_t solidRow = (1<<10)-1;

	/// compactRowAt returns the compactField value at row.
	uint16_t compactRowAt(int y) const {
		if(y < 0) return solidRow;
		if(compactFields.size() <= y) return 0;
		return compactFields[y];
	}

	/// rowAt retrieves the row vector with specified index.
	fieldRow rowAt(int y, uint8_t solidCell = 1) const {
		if(compactFields.size() <= y) return fieldRow();
		else if(y < 0) {
			fieldRow result;
			for(int i = 0; i < 10; ++ i) result[i] = solidCell;
			return result;
		}
		return fields[y];
	}

	/// move is used to move on the horizontal direction with
	/// specified steps, returning whether it could eventually
	/// find a tile on that direction.
	bool move(const tilePathFinder& pfd,
		int8_t numSteps, tilePathFinder& result) const;

	/// drop is used to move on the vertical direction with
	/// specified steps, returning whether it could eventually
	/// move onto some tile on that direction.
	bool drop(const tilePathFinder& pfd,
		uint8_t numSteps, tilePathFinder& result) const;

	/// rotate attempt to perform rotation on the given
	/// tile, and output whether the rotation could be done.
	bool rotate(const tilePathFinder& pfd,
		tileDirection targetDir, tilePathFinder& result) const;

	/// lock will eventually place a tile on the tracker
	/// location, modify the state of the field.
	bool lock(const tilePathFinder& pfd, uint8_t& clear);

	/// grow add tiles to the bottom of the fields.
	void grow(fieldRow row);
}; // struct hacktile::model::field

} // namespace hacktile::model
} // namespace hacktile
