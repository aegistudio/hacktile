# SPDX-License-Identifier: LGPL-3.0-or-later
# HackTile Testing Setup Definition
include(CTest)
enable_testing()

# Find GoogleTest for unit testing, some handy macros are
# also defined here.
find_package(GTest)
if(NOT GTEST_FOUND)
	message(SEND_ERROR "GoogleTest is required")
endif()
# hacktile_add_test(${NAME} FILES ${FILES} LINKS ${LINKS})
function(hacktile_add_test NAME)
	# Collect and create test arguments.
	set(CURRENT_FILES_STRIP 0)
	set(FILES)
	set(CURRENT_LINKS_STRIP 0)
	set(LINKS)
	foreach(arg IN LISTS ARGN)
		if("${arg}" STREQUAL "FILES")
			set(CURRENT_FILES_STRIP 1)
			set(CURRENT_LINKS_STRIP 0)
		elseif("${arg}" STREQUAL "LINKS")
			set(CURRENT_FILES_STRIP 0)
			set(CURRENT_LINKS_STRIP 1)
		elseif(${CURRENT_FILES_STRIP} EQUAL 1)
			list(APPEND FILES ${arg})
		elseif(${CURRENT_LINKS_STRIP} EQUAL 1)
			list(APPEND LINKS ${arg})
		else()
			message(SEND_ERROR "argument ${arg} out of scope")
			return()
		endif()
	endforeach()
	add_executable(${NAME} ${FILES})
	target_link_libraries(${NAME} GTest::GTest GTest::Main ${LINKS})
	target_include_directories(${NAME} PRIVATE ${GTEST_INCLUDE_DIRS})
	add_test(${NAME} ${NAME})
endfunction()
