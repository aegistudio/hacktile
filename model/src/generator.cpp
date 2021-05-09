// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file generator.cpp
 * @author aegistudio
 * @brief Implementation of some tile generator.
 *
 * This file implements the tile permutator, with C++ pseudo
 * randomizer and full permutate algorithm.
 */
#include "model/generator.hpp"
#include <algorithm>
#include <random>

namespace hacktile {
namespace model {

// forward the definition of the commonly used randomizer.
typedef std::mt19937 randomizer;

void tilePermutator::permutate() {
	// Randomize the series for the first round.
	std::shuffle(&series[0], &series[numTiles],
		*reinterpret_cast<randomizer*>(workData));
}

tilePermutator::tilePermutator(
	const tile* tiles[], size_t numTiles, uint64_t seed):
	series(new const tile*[numTiles]),
	numTiles(numTiles), pointer(0) {

	// Initialize the series and random generator.
	for(size_t i = 0; i < numTiles; ++ i)
		series[i] = tiles[i];
	static_assert(sizeof(randomizer) <= sizeof(workData),
		"not enough space to hold the random generator");
	new (workData) randomizer(seed);

	// Randomize the series for the first round.
	permutate();
}

tilePermutator::~tilePermutator() {
	reinterpret_cast<randomizer*>(workData)->~randomizer();
}

const tile* tilePermutator::generate() {
	const tile* result = series[pointer];
	++ pointer;
	if(pointer >= numTiles) {
		pointer = 0;
		permutate();
	}
	return result;
}

historyRoll::historyRoll(
	const tile* tiles_[], std::size_t numTiles,
	std::size_t retryTimes, std::size_t* initialHistory,
	std::size_t historySize, uint64_t seed):
	history(initialHistory, initialHistory + historySize),
	counts(new std::size_t[numTiles]),
	tiles(new const tile*[numTiles]), numTiles(numTiles),
	retryTimes(retryTimes), historySize(historySize) {
	std::copy(tiles_, tiles_ + numTiles, tiles.get());
	std::fill_n(counts.get(), numTiles, 0);
	for (std::size_t i : history)
		if (i < numTiles)
			counts[i]++;
	static_assert(sizeof(randomizer) <= sizeof(workData),
		"not enough space to hold the random generator");
	new (workData) randomizer(seed);
}

historyRoll::~historyRoll() {
	reinterpret_cast<randomizer*>(workData)->~randomizer();
}

const tile* historyRoll::generate() {
	std::uniform_int_distribution<std::size_t> sampler{0, numTiles - 1};
	std::size_t result = sampler(*reinterpret_cast<randomizer*>(workData));
	for (std::size_t i = 0; i < retryTimes && counts[result] > 0; ++i)
		result = sampler(*reinterpret_cast<randomizer*>(workData));
	history.push_back(result);
	counts[result]++;
	if (history.front() < numTiles)
		counts[history.front()]--;
	history.pop_front();
	return tiles[result];
}

} // namespace hacktile::model
} // namespace hacktile
