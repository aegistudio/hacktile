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

} // namespace hacktile::model
} // namespace hacktile
