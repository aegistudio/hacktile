#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file generator.hpp
 * @brief tile generator interface and most known implementation
 * @author aegistudio
 *
 * This file provides the interface definition of tile generator
 * providing tiles mainly for the playground. It also provides
 * the well known implementation known as permutator, which always
 * select tiles from a set of tiles and return it to caller.
 */
#include "model/tile.hpp"
#include <memory>
#include <deque>

namespace hacktile {
namespace model {

/**
 * @brief tileGenerator is the interface to provide tile.
 */
struct tileGenerator {
	// virtual destructor for pure virtual class.
	virtual ~tileGenerator() {}

	/// generate next tile for processing.
	///
	/// If the generator returns nullptr, it will be considered
	/// as tile exhausted and after all tiles previously
	/// generated has been used up, playground will generate
	/// an tileExhaustEvent.
	virtual const tile* generate() = 0;
};

/**
 * @brief tilePermutator is a tile generator that randomize all
 * tiles but ensures that all tiles appears in the next series.
 *
 * The tilePermutator has a pseudo randomizer imbued inside and
 * given the same seed it and input series, it will always yield
 * the same result.
 */
class tilePermutator : public tileGenerator {
	std::unique_ptr<const tile*[]> series;
	size_t numTiles;
	int pointer;
	char workData[5000];
	void permutate();
public:
	/// default constructor for the permutator.
	tilePermutator(const tile* tiles[], size_t numTiles, uint64_t seed);

	/// virtual destructor for the permutator.
	virtual ~tilePermutator();

	/// generate method implementation of permutator.
	virtual const tile* generate();
};

/**
 * @brief historyRoll is a tile generator that will randomly pick a piece
 * and try keeping the new piece different from the older ones.
 * 
 * It will remember `historySize` pieces, and retry `retryTimes` times before
 * it gives up and just generate a piece ignoring the history.
 * 
 * XXX: Consider tile sequence `A A B` where `A`s are the same pointer;
 * what should we do if we randomly pick `A1` but there is an `A2` in history?
 * Currently we think `A1` and `A2` are different, but if we change `history`
 * into `tiles *`, `A1` and `A2` will be treated as the same pieces.
 */
class historyRoll: public tileGenerator {
	std::deque<std::size_t> history;
	std::unique_ptr<std::size_t[]> counts;
	// TODO: in fact only a readable *constant* copy of tiles is needed,
	// so if we can make sure `tiles` will outlive this `class`,
	// we should make it a pointer and avoid hard copying pointers.
	std::unique_ptr<const tile*[]> tiles;
	std::size_t numTiles;
	const std::size_t retryTimes, historySize;
	char workData[5000];
public:
	/**
	 * @brief Construct a new history Roll object
	 * 
	 * @param tiles_ tiles to choose from
	 * @param numTiles size of tiles
	 * @param retryTimes how many times it should roll before giving up
	 * @param initialHistory indices of tiles in previous history,
	 * from oldest to newest.  For example, if `tiles` are `A B`,
	 * a history of `0 1 1` means previous history are `A A B`.
	 * Out-of-index history will not match any newly-generated tile.
	 * @param historySize size of history
	 * @param seed seed for the random number generator
	 */
	historyRoll(
		const tile* tiles_[], std::size_t numTiles,
		std::size_t retryTimes, std::size_t* initialHistory,
		std::size_t historySize, uint64_t seed);

	~historyRoll() override;

	const tile* generate() override;
};

} // namespace hacktile::model
} // namespace hacktile
