#
# CMakeLists - pointless frippery & tremendous amounts of bloat
# Copyright (c) 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
#

if (MINGW)

set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++ CACHE STRING "C++ compiler" FORCE)
set(project "create_shortcut")
project(${project} LANGUAGES CXX)
set_source_files_properties(${project}.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(${project}.c PROPERTIES LANGUAGE CXX)
set_source_files_properties(winfrip_rtl.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra -DWINFRIP_RTL_NO_PCRE2")
set_source_files_properties(winfrip_rtl.c PROPERTIES LANGUAGE CXX)
set_source_files_properties(winfrip_rtl_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(winfrip_rtl_file.c PROPERTIES LANGUAGE CXX)
set_source_files_properties(winfrip_rtl_shortcut.cxx PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(winfrip_rtl_shortcut.cxx PROPERTIES LANGUAGE CXX)
add_executable(${project} ${project}.c winfrip_rtl.c winfrip_rtl_file.c winfrip_rtl_shortcut.cxx)
set_target_properties(${project} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${project} PROPERTIES WIN32_EXECUTABLE ON LINK_FLAGS "${LFLAG_MANIFEST_NO}")
target_link_libraries(${project} ${platform_libraries} shlwapi.lib)
installed_program(${project})

endif()

#
# vim:noexpandtab sw=8 ts=8 tw=0
#
