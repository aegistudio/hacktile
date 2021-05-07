# SPDX-License-Identifier: LGPL-3.0-or-later
# hacktile - HackTile Game Project
cmake_minimum_required(VERSION 3.10)
project(hacktile C CXX)

# Setup mandatory CMake configurations.
set(CMAKE_CXX_STANDARD 11) # C++11 Required
LIST(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(HacktileTesting)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# Aggregate all CMake module components here.
add_subdirectory(util)
add_subdirectory(model)
add_subdirectory(terminal)
