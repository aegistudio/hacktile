# SPDX-License-Identifier: LGPL-3.0-or-later
# HackTile Command Line Interface View

# Specify hackTileModel.a|lib static library build instruction.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/util/include")
include_directories("${CMAKE_SOURCE_DIR}/model/include")
add_library(hacktileViewCli STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/terminal.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tile.cpp")

# Build test binaries and specify test cases.
