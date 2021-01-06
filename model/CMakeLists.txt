# SPDX-License-Identifier: LGPL-3.0-or-later
# HackTile Model Module

# Specify hackTileModel.a|lib static library build instruction.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/util/include")
add_library(hacktileModel STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tile.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tetromino.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/playground.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/generator.cpp")

# Build test binaries and specify test cases.
hacktile_add_test(hacktileModelTest FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/tests/tile.cpp"
	LINKS hacktileModel)
