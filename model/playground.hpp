#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file playground.hpp
 * @brief generic interface for a player's main playground
 * @author aegistudio
 *
 * This file provides core model aiming at providing dynamic
 * and client (CLI, GUI, etc.) independent information that
 * could be viewed by the user, or orchestrated in scenarios.
 */
#include "util/event.hpp"
#include "model/tile.hpp"
#include "model/generator.hpp"
#include <memory>

namespace hacktile {
namespace model {

/**
 * @brief tileSpawnEvent is triggered when a tile has
 * been spawned in the field.
 */
struct tileSpawnEvent {
	const tile& type;
	tileState location, locationShadow;
};

/**
 * @brief tileMoveEvent is triggered when a tile in the
 * field has been moved.
 */
struct tileMoveEvent {
	const tile& type;
	tileState before, beforeShadow;
	tileState after, afterShadow;
	bool wallKick;
};

/**
 * @brief tileBeforeLockEvent is triggered when a tile
 * has reached lock condition and about to evaluate
 * tile locking soon.
 *
 * The subscriber could always expect a tileLock
 * invocation soon after the event happens.
 */
struct tileBeforeLockEvent {
	const tile& type;
	tileState location;
};

/**
 * @brief tileLockEvent is triggered when a tile in the
 * field has been locked and potentially erase lines.
 */
struct tileLockEvent {
	const tile& type;
	tileState location;
	uint8_t clear;
};

/**
 * @brief tileSwapEvent is triggered when a tile in the
 * field has been swapped out and placed in the next.
 *
 * The behaviour of a swap event must seems as if the
 * current tile has disappered but later shown up with
 * a tileSpawnEvent.
 */
struct tileSwapEvent {
	const tile& type;
	tileState location, locationShadow;
};

/**
 * @brief playgroundState is represents the game state
 * of a playground.
 *
 * Except in the state playgroundState::inGame, all other
 * input will be ignored.
 */
enum class playgroundState {
	notStarted,
	inGame,
	topOut,
	exhausted,
	completed,
};

/**
 * @brief gameEndEvent is triggered when the playground
 * has reaches one of the termination state.
 */
struct gameEndEvent {
	playgroundState endState;
};

/**
 * @brief playgroundListener is the decoupled event handler
 * that is registered to the playground and notified once
 * one of the topics have been changed.
 *
 * XXX: all handlers are guaranteed to be invoked when the
 * modification has already been applied to the lowest level
 * playground object, so be sure to capture the event input
 * right in the controller if you want to filter operations.
 */
struct playgroundListener {
	// virtual destructor for virtual handler class.
	virtual ~playgroundListener() {}

	/// tileSpawn is triggered when a tile should enter
	/// the current field.
	virtual void tileSpawn(const tileSpawnEvent&) {}

	/// tileMove is triggered when a tile state has been
	/// updated. The previous and new state will be evaluated.
	virtual void tileMove(const tileMoveEvent&) {}

	/// tileBeforeLock is triggered when a tile has reached
	/// locked condition but not yet locked. This is exposed
	/// by the playground for listeners to evaluate current
	/// type of erasure.
	virtual void tileBeforeLock(const tileBeforeLockEvent&) {}

	/// tileLock is triggered when a tile has locked and
	/// potentially some lines has been erased. Some
	/// tile erasure judgement will also be performed.
	///
	/// The next tile will be swapped in through tileSpawn.
	virtual void tileLock(const tileLockEvent&) {}

	/// tileSwap is triggered when a currently held has been
	/// altered swapped out. The next tile will be swapped
	/// in through tileSpawn.
	virtual void tileSwap(const tileSwapEvent&) {}

	/// gameEnd is triggered when the game has reached
	/// a loss or complete condition. And no more event will
	/// be generated since then.
	virtual void gameEnd(const gameEndEvent&) {}
}; // struct hacktile::model::playgroundListener

/**
 * @brief playground is complete neutral abstract playground
 * that could be used in various of games.
 *
 * A playground represents a controlled field that could
 * generate tiles and accept from the controller, which
 * is associated with user input or AI moves.
 */
class playground :
	public hacktile::util::eventRegistry<playgroundListener> {

	field f;
	tileGenerator* generator;
	const tile* swap;
	bool swapEnabled;
	tilePathFinder current, shadow;
	int numPreviews;
	std::unique_ptr<const tile*[]> preview;
	int previewCursor;
	playgroundState state;

	// spawnNextTile will attempt to spawn the next tile into
	// the game and update the preview series.
	void spawnNextTile();

	// spawnTile will spawn a tile of specified type in the
	// field (might overlap with field) and determine whether
	// the field has topped out.
	void spawnTile(const tile* typ);

	// tileMove will update the tile state with the newly
	// evaluated tile state.
	void tileMove(tilePathFinder pfd);

	// rotate is the function handling the rotation.
	bool rotate(tileDirection newDir);
public:
	/// default constructor for creating current playground.
	playground(tileGenerator* generator, int numPreviews = 5);

	/// start will attempt to start the game.
	///
	/// If the game has already started, invoking the function
	/// again will result in exception.
	void start();

	/// complete will attempt to complete the game.
	///
	/// The playground will not know whether the player has
	/// success or fail, it just stops the playground from
	/// receiving more input.
	///
	/// If the tile has already spawned in field, it will
	/// remains in their location.
	void complete();

	/// getState returns the current game state.
	playgroundState getState() const {
		return state;
	}

	/// isInGame returns whether it is currently in game state.
	bool isInGame() const {
		return state == playgroundState::inGame;
	}

	/// getField returns the current handle of field.
	const field& getField() const {
		return f;
	}

	/// getSwapTile returns the current tile in swap.
	const tile* getSwapTile() const {
		return swap;
	}

	/// isSwapEnabled returns whether swap is enabled now.
	bool isSwapEnabled() const {
		return swapEnabled;
	}

	/// getCurrentTile returns the type of current tile.
	const tile* getCurrentTile() const {
		return current.getType();
	}

	/// getCurrentState returns the state of current tile.
	const tileState& getCurrentState() const {
		return current.getState();
	}

	/// getShadowState returns the state of shadow tile.
	const tileState& getShadowState() const {
		return shadow.getState();
	}

	/// getNumPreviews returns the number of previews.
	int getNumPreviews() const {
		return numPreviews;
	}

	/// getPreview returns the preview at index.
	const tile* getPreview(int i) const {
		if(i < 0 || i >= numPreviews) return nullptr;
		return preview[(previewCursor + i) % numPreviews];
	}

	/// move will attempt to move the tile.
	bool move(int8_t dx);

	/// drop will attempt to drop the tile. Please notice
	/// dropping manually will not lock the tile, unlike the
	/// drop imposed by naturalDrop.
	bool drop(uint8_t dy);

	/// hardDrop will move the tile to bottom and lock tile.
	///
	/// Lock delay handlers will also invoke this method, and
	/// since it is possible to invoke this method from event
	/// handlers, the controller / renderer might find that
	/// the tile actually locked when returned.
	bool hardDrop();

	/// rotateCCW will attempt to rotate the tile CCW.
	bool rotateCCW() {
		if(!isInGame()) return false;
		return rotate(getCurrentState().dir.rotateCCW());
	}

	/// rotateCW will attempt to rotate the tile CW.
	bool rotateCW() {
		if(!isInGame()) return false;
		return rotate(getCurrentState().dir.rotateCW());
	}

	/// halfTurn will attempt to half turn the tile.
	bool halfTurn() {
		if(!isInGame()) return false;
		return rotate(getCurrentState().dir.halfTurn());
	}

	/// swapTile will attempt to swap the current tile.
	bool swapTile();
}; // struct hacktile::model::playground

} // namespace hacktile::model
} // namespace hacktile
