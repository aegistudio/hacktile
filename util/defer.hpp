#pragma once
// SPDX-License-Identifier: LGPL-3.0-or-later
/**
 * @file defer.hpp
 * @brief the defer function definition helper
 * @author aegistudio
 *
 * This file provides the interface for executing registered
 * defer function when it goes out of scope. This should be
 * handy when you want to recycle objects in place.
 */
#include <functional>

namespace hacktile {
namespace util {

class defer {
	std::function<void()> f;
public:
	defer(std::function<void()>&& f): f(std::move(f)) {}

	~defer() {
		if(f) f();
	}

	void release() noexcept {
		std::function<void()> empty;
		std::swap(f, empty);
	}
}; // class hacktile::util::defer

} // namespace hacktile::util
} // namespace hacktile
