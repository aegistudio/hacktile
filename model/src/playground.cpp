// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file playground
 * @author aegistudio
 * @brief Implementation of the playground core.
 *
 * This file implements the logic of playground, which tracks
 * the current gameplay state, provides data for renderers and
 * notifies renderer and controller about updates.
 */
#include "model/playground.hpp"

namespace hacktile {
namespace model {

playground::playground(tileGenerator* generator, int numPreviews):
	f(), generator(generator), swap(nullptr), swapEnabled(true),
	current(), shadow(), preview(new const tile*[numPreviews]),
	numPreviews(numPreviews), previewCursor(0),
	state(playgroundState::notStarted) {

	// Initalize the initial preview tiles in the playground.
	for(int i = 0; i < numPreviews; ++ i)
		preview[i] = generator->generate();
}

void playground::spawnNextTile() {
	// Retrieve the next tile to spawn.
	const tile* currentType = preview[previewCursor];
	if(currentType == nullptr) {
		current = tilePathFinder();
		shadow = tilePathFinder();
		state = playgroundState::exhausted;

		gameEndEvent event = {
			.endState = state,
		};
		dispatch(&playgroundListener::gameEnd, event);
		return;
	}

	// Attempt to generate the next tile based on whether
	// the previous tile is nullptr.
	const tile* previous = nullptr;
	if(previewCursor == 0)
		previous = preview[numPreviews-1];
	else previous = preview[previewCursor-1];
	const tile* next = nullptr;
	if(previous != nullptr)
		next = generator->generate();
	preview[previewCursor] = next;
	++previewCursor;
	if(previewCursor >= numPreviews) previewCursor = 0;

	// Spawn the current tile in the field.
	spawnTile(currentType);
}

void playground::spawnTile(const tile* typ) {
	current = tilePathFinder(typ);
	shadow = tilePathFinder(typ);
	if(!f.spawn(current)) {
		// Update the game state to render that the current
		// game has topped out.
		current = tilePathFinder(typ);
		state = playgroundState::topOut;

		// Notify about the tile spawn event so that some
		// will see the newly spawned tile overlaps with
		// the current ones.
		tileSpawnEvent spawnEvent = {
			.type           = *current.getType(),
			.location       = current.getState(),
			.locationShadow = shadow.getState(),
		};
		dispatch(&playgroundListener::tileSpawn, spawnEvent);

		// Notify about that the game has topped out.
		gameEndEvent endEvent = {
			.endState = state,
		};
		dispatch(&playgroundListener::gameEnd, endEvent);
		return;
	}

	// The tile can be spawn in the field, so we will attempt
	// to move the shadow tile to its location.
	tilePathFinder newShadow;
	if(f.drop(current, 20, newShadow)) {
		std::swap(shadow, newShadow);
	}

	// Notify the subscribers about the tile spawn.
	tileSpawnEvent spawnEvent = {
		.type           = *current.getType(),
		.location       = current.getState(),
		.locationShadow = shadow.getState(),
	};
	dispatch(&playgroundListener::tileSpawn, spawnEvent);
	return;
}

void playground::start() {
	if(state != playgroundState::notStarted) return;
	state = playgroundState::inGame;
	spawnNextTile();
}

void playground::complete() {
	if(state != playgroundState::inGame) return;
	state = playgroundState::completed;
	gameEndEvent endEvent = {
		.endState = state,
	};
	dispatch(&playgroundListener::gameEnd, endEvent);
}

void playground::tileMove(tilePathFinder pfd) {
	std::swap(current, pfd);
	tilePathFinder newShadow;
	if(!f.drop(current, 20, newShadow))
		newShadow = current;
	std::swap(shadow, newShadow);
	tileMoveEvent moveEvent = {
		.type         = *current.getType(),
		// Since the pfd-current, shadow-newShadow are swapped
		// which means the pfd and newShadow stores the older
		// state, while current and shadow stores the newer.
		.before       = pfd.getState(),
		.beforeShadow = newShadow.getState(),
		.after        = current.getState(),
		.afterShadow  = shadow.getState(),
		.wallKick     = current.isPreviousWallKick(),
	};
	dispatch(&playgroundListener::tileMove, moveEvent);
}

bool playground::move(int8_t movement) {
	if(!isInGame()) return false;
	tilePathFinder pfd;
	if(!f.move(current, movement, pfd)) return false;
	tileMove(std::move(pfd));
	return true;
}

bool playground::drop(uint8_t movement) {
	if(!isInGame()) return false;
	tilePathFinder pfd;
	if(!f.drop(current, movement, pfd)) return false;
	tileMove(std::move(pfd));
	return true;
}

bool playground::rotate(tileDirection newDir) {
	if(!isInGame()) return false;
	tilePathFinder pfd;
	if(!f.rotate(current, newDir, pfd)) return false;
	tileMove(std::move(pfd));
	return true;
}

bool playground::hardDrop() {
	// Attempt to drop 20 tiles so that tileMove will be
	// notified if the tile could be dropped from middle air.
	if(!isInGame()) return false;
	drop(20);
	const tile& type = *current.getType();
	tileState location = current.getState();

	// Now the tile is moved and is on the ground, dispatch
	// the before lock event to trigger calculation.
	tileBeforeLockEvent beforeEvent = {
		.type     = type,
		.location = location,
	};
	dispatch(&playgroundListener::tileBeforeLock, beforeEvent);

	// Attempt to lock the tile and calculate line clears.
	uint8_t clear = 0;
	f.lock(current, clear);
	current = tilePathFinder();
	shadow = tilePathFinder();
	swapEnabled = true;

	// Dispatch the tile lock and clear event.
	tileLockEvent afterEvent = {
		.type     = type,
		.location = location,
		.clear    = clear,
	};
	dispatch(&playgroundListener::tileLock, afterEvent);

	// We will also attempt to spawn the next tile, as the
	// natural behaviour of a game.
	if(isInGame()) spawnNextTile();
	return true;
}

bool playground::swapTile() {
	if(!isInGame()) return false;
	if(!swapEnabled) return false;

	// Swap the current piece into stack and generate event.
	const tile* typ = current.getType();
	const tile* previous = swap;
	tileState location = current.getState();
	tileState locationShadow = shadow.getState();
	current = tilePathFinder();
	shadow = tilePathFinder();
	swap = typ;
	swapEnabled = previous == nullptr;

	// Dispatch the tile swap event at first.
	tileSwapEvent swapEvent = {
		.type           = *typ,
		.location       = location,
		.locationShadow = locationShadow,
	};
	dispatch(&playgroundListener::tileSwap, swapEvent);

	// Spawn the previous tile or next tile.
	if(previous == nullptr)
		spawnNextTile();
	else spawnTile(previous);
	return true;
}

} // namespace hacktile::model
} // namespace hacktile
