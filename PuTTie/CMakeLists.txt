#
# CMakeLists - pointless frippery & tremendous amounts of bloat
# Copyright (c) 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
#

if (MINGW)

set(project "create_shortcut")
set_source_files_properties(${project}.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(winfrip_rtl.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra -DWINFRIP_RTL_NO_PCRE2")
set_source_files_properties(winfrip_rtl_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(winfrip_rtl_shortcut.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
add_executable(${project} ${project}.c winfrip_rtl.c winfrip_rtl_file.c winfrip_rtl_shortcut.c)
set_target_properties(${project} PROPERTIES WIN32_EXECUTABLE ON LINK_FLAGS "${LFLAG_MANIFEST_NO}")
target_link_libraries(${project} ${platform_libraries} shlwapi.lib)
installed_program(${project})

endif()

#
# vim:noexpandtab sw=8 ts=8 tw=0
#
