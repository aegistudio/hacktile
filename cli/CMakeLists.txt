# SPDX-License-Identifier: LGPL-3.0-or-later
# HackTile CLI Application

# Build the main executable by specification.
include_directories("${CMAKE_SOURCE_DIR}/model/include")
include_directories("${CMAKE_SOURCE_DIR}/view/cli/include")
include_directories("${CMAKE_SOURCE_DIR}/util/include")
add_executable(hacktile-cli
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(
	hacktile-cli hacktileModel hacktileViewCli)
