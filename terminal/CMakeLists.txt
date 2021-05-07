# SPDX-License-Identifier: LGPL-3.0-or-later
# HackTile Terminal Subsystem

# Specify hacktileTerminal.a|lib library.
add_library(hacktileTerminalBase STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/terminal.cpp")

# Specify hackTileTerminalView.a|lib library.
add_library(hacktileTerminalView STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/view/tile.cpp")

# Build the main executable by specification.
add_executable(hacktile-cli
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(hacktile-cli
	hacktileModel hacktileTerminalBase hacktileTerminalView)
