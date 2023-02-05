set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_sources_from_current_dir(utils
  utils/agent_mutex_name.c
  utils/agent_named_pipe_name.c
  utils/arm_arch_queries.c
  utils/aux_match_opt.c
  utils/centre_window.c
  utils/cryptoapi.c
  utils/defaults.c
  utils/dll_hijacking_protection.c
  utils/dputs.c
  utils/escape_registry_key.c
  utils/filename.c
  utils/fontspec.c
  utils/getdlgitemtext_alloc.c
  utils/get_system_dir.c
  utils/get_username.c
  utils/gui-timing.c
  utils/interprocess_mutex.c
  utils/is_console_handle.c
  utils/load_system32_dll.c
  utils/ltime.c
  utils/makedlgitemborderless.c
  utils/make_spr_sw_abort_winerror.c
  utils/message_box.c
  utils/minefield.c
  utils/open_for_write_would_lose_data.c
  utils/pgp_fingerprints_msgbox.c
  utils/platform_get_x_display.c
  utils/registry.c
  utils/request_file.c
  utils/screenshot.c
  utils/security.c
  utils/shinydialogbox.c
  utils/split_into_argv.c
  utils/version.c
  utils/win_strerror.c
  unicode.c)
if(NOT HAVE_STRTOUMAX)
  add_sources_from_current_dir(utils utils/strtoumax.c)
endif()
add_sources_from_current_dir(eventloop
  cliloop.c handle-wait.c)
add_sources_from_current_dir(console
  select-cli.c nohelp.c console.c)
add_sources_from_current_dir(settings
  storage.c)
add_sources_from_current_dir(network
  network.c handle-socket.c named-pipe-client.c named-pipe-server.c local-proxy.c x11.c)
add_sources_from_current_dir(sshcommon
  noise.c)
add_sources_from_current_dir(sshclient
  agent-client.c gss.c sharing.c)
add_sources_from_current_dir(sftpclient
  sftp.c)
add_sources_from_current_dir(otherbackends
  serial.c)
add_sources_from_current_dir(agent
  agent-client.c)
add_sources_from_current_dir(guiterminal
  dialog.c controls.c config.c printing.c jump-list.c sizetip.c)
add_dependencies(guiterminal generated_licence_h) # dialog.c uses licence.h

# This object awkwardly needs to live in the network library as well
# as the eventloop library, in case it didn't get pulled in from the
# latter before handle-socket.c needed it.
add_library(handle-io OBJECT
  handle-io.c)
target_sources(eventloop PRIVATE $<TARGET_OBJECTS:handle-io>)
target_sources(network PRIVATE $<TARGET_OBJECTS:handle-io>)

add_library(guimisc STATIC
  select-gui.c)

# {{{ winfrip
if (MINGW)
set_source_files_properties(../PuTTie/winfrip_feature_bgimg.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_general.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_mouse.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_storage_general.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_storage_host_ca.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_storage_host_keys.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_storage_priv.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_storage_sessions.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_trans.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_feature_urls.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_debug.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_gdi.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_pcre2.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_registry.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_serialise.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_terminal.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_rtl_tree.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_adapter.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_adapter_host_ca.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_adapter_host_keys.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_adapter_sessions.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_backend_ephemeral.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_backend_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_backend_registry.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_host_ca.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_host_keys.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_jump_list.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_options.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_priv.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_privkey_list.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_storage_sessions.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_subr_pageant.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(../PuTTie/winfrip_subr_putty_config.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")

if ("${DEFAULT_STORAGE_BACKEND}" STREQUAL "")
  set(DEFAULT_STORAGE_BACKEND "WFS_BACKEND_REGISTRY")
endif()
set_property(SOURCE ../PuTTie/winfrip_storage.c APPEND_STRING PROPERTY COMPILE_FLAGS " -DWFS_DEFAULT_STORAGE_BACKEND=${DEFAULT_STORAGE_BACKEND}")

if ("${BUILD_SSH_VERSION}" STREQUAL "")
  set(BUILD_SSH_VERSION "-Unidentified-Local-Build")
endif()
if ("${BUILD_TEXT_VERSION}" STREQUAL "")
  set(BUILD_TEXT_VERSION "Unidentified build")
endif()
set_property(SOURCE utils/version.c APPEND_STRING PROPERTY COMPILE_FLAGS " -DSSHVER=\"\\\"${BUILD_SSH_VERSION}\\\"\" -DTEXTVER=\"\\\"${BUILD_TEXT_VERSION}\\\"\"")

endif()
# winfrip }}}

add_executable(pageant
  pageant.c
  help.c
  pageant.rc)
# {{{ winfrip
if (MINGW)
set_property(TARGET pageant APPEND PROPERTY SOURCES
  ../PuTTie/no-jump-list.c
  ../PuTTie/no-jump-list-pageant.c
  ../PuTTie/winfrip_rtl.c
  ../PuTTie/winfrip_rtl_debug.c
  ../PuTTie/winfrip_rtl_file.c
  ../PuTTie/winfrip_rtl_pcre2.c
  ../PuTTie/winfrip_rtl_registry.c
  ../PuTTie/winfrip_rtl_serialise.c
  ../PuTTie/winfrip_rtl_tree.c
  ../PuTTie/winfrip_storage.c
  ../PuTTie/winfrip_storage_adapter.c
  ../PuTTie/winfrip_storage_adapter_host_ca.c
  ../PuTTie/winfrip_storage_adapter_host_keys.c
  ../PuTTie/winfrip_storage_adapter_sessions.c
  ../PuTTie/winfrip_storage_backend_ephemeral.c
  ../PuTTie/winfrip_storage_backend_file.c
  ../PuTTie/winfrip_storage_backend_registry.c
  ../PuTTie/winfrip_storage_host_ca.c
  ../PuTTie/winfrip_storage_host_keys.c
  ../PuTTie/winfrip_storage_jump_list.c
  ../PuTTie/winfrip_storage_options.c
  ../PuTTie/winfrip_storage_priv.c
  ../PuTTie/winfrip_storage_privkey_list.c
  ../PuTTie/winfrip_storage_sessions.c
  ../PuTTie/winfrip_subr_pageant.c)
endif()
# winfrip }}}
add_dependencies(pageant generated_licence_h)
target_link_libraries(pageant
  guimisc eventloop agent network crypto utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2@master/libpcre2-16.a
  ${platform_libraries})
set_target_properties(pageant PROPERTIES
  WIN32_EXECUTABLE ON
  LINK_FLAGS "${LFLAG_MANIFEST_NO}")
installed_program(pageant)

add_sources_from_current_dir(plink no-jump-list.c nohelp.c plink.rc)
add_dependencies(plink generated_licence_h)

add_sources_from_current_dir(pscp no-jump-list.c nohelp.c pscp.rc)
add_dependencies(pscp generated_licence_h)

add_sources_from_current_dir(psftp no-jump-list.c nohelp.c psftp.rc)
add_dependencies(psftp generated_licence_h)

add_sources_from_current_dir(psocks nohelp.c)

add_executable(putty
  window.c
  putty.c
  help.c
  ${CMAKE_SOURCE_DIR}/stubs/no-console.c
  putty.rc)
# {{{ winfrip
if (MINGW)
set_property(TARGET putty APPEND PROPERTY SOURCES
  ../PuTTie/winfrip_feature_bgimg.c
  ../PuTTie/winfrip_feature_general.c
  ../PuTTie/winfrip_feature_mouse.c
  ../PuTTie/winfrip_feature_storage_general.c
  ../PuTTie/winfrip_feature_storage_host_ca.c
  ../PuTTie/winfrip_feature_storage_host_keys.c
  ../PuTTie/winfrip_feature_storage_priv.c
  ../PuTTie/winfrip_feature_storage_sessions.c
  ../PuTTie/winfrip_feature_trans.c
  ../PuTTie/winfrip_feature_urls.c
  ../PuTTie/winfrip_rtl.c
  ../PuTTie/winfrip_rtl_debug.c
  ../PuTTie/winfrip_rtl_file.c
  ../PuTTie/winfrip_rtl_gdi.c
  ../PuTTie/winfrip_rtl_pcre2.c
  ../PuTTie/winfrip_rtl_registry.c
  ../PuTTie/winfrip_rtl_serialise.c
  ../PuTTie/winfrip_rtl_terminal.c
  ../PuTTie/winfrip_rtl_tree.c
  ../PuTTie/winfrip_storage.c
  ../PuTTie/winfrip_storage_adapter.c
  ../PuTTie/winfrip_storage_adapter_host_ca.c
  ../PuTTie/winfrip_storage_adapter_host_keys.c
  ../PuTTie/winfrip_storage_adapter_sessions.c
  ../PuTTie/winfrip_storage_backend_ephemeral.c
  ../PuTTie/winfrip_storage_backend_file.c
  ../PuTTie/winfrip_storage_backend_registry.c
  ../PuTTie/winfrip_storage_host_ca.c
  ../PuTTie/winfrip_storage_host_keys.c
  ../PuTTie/winfrip_storage_jump_list.c
  ../PuTTie/winfrip_storage_options.c
  ../PuTTie/winfrip_storage_priv.c
  ../PuTTie/winfrip_storage_privkey_list.c
  ../PuTTie/winfrip_storage_sessions.c
  ../PuTTie/winfrip_subr_putty_config.c)
endif()
# winfrip }}}
be_list(putty PuTTie SSH SERIAL OTHERBACKENDS)
add_dependencies(putty generated_licence_h)
target_link_libraries(putty
  guiterminal guimisc eventloop sshclient otherbackends settings network crypto
  utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2@master/libpcre2-16.a
  gdiplus.lib msimg32.lib
  bcrypt.lib
  ${platform_libraries})
if (MINGW)
  target_link_options(putty PRIVATE
    -Wl,-Bstatic -l:libwinpthread.a)
endif()
set_target_properties(putty PROPERTIES
  WIN32_EXECUTABLE ON
  LINK_FLAGS "${LFLAG_MANIFEST_NO}")
installed_program(putty)

add_executable(puttytel
  window.c
  putty.c
  help.c
  ${CMAKE_SOURCE_DIR}/stubs/no-gss.c
  ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.c
  ${CMAKE_SOURCE_DIR}/stubs/no-console.c
  ${CMAKE_SOURCE_DIR}/stubs/no-rand.c
  ${CMAKE_SOURCE_DIR}/proxy/nocproxy.c
  ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.c
  puttytel.rc)
# {{{ winfrip
if (MINGW)
set_property(TARGET puttytel APPEND PROPERTY SOURCES
  ../PuTTie/winfrip_feature_bgimg.c
  ../PuTTie/winfrip_feature_general.c
  ../PuTTie/winfrip_feature_mouse.c
  ../PuTTie/winfrip_feature_storage_general.c
  ../PuTTie/winfrip_feature_storage_host_ca.c
  ../PuTTie/winfrip_feature_storage_host_keys.c
  ../PuTTie/winfrip_feature_storage_priv.c
  ../PuTTie/winfrip_feature_storage_sessions.c
  ../PuTTie/winfrip_feature_trans.c
  ../PuTTie/winfrip_feature_urls.c
  ../PuTTie/winfrip_rtl.c
  ../PuTTie/winfrip_rtl_debug.c
  ../PuTTie/winfrip_rtl_file.c
  ../PuTTie/winfrip_rtl_gdi.c
  ../PuTTie/winfrip_rtl_pcre2.c
  ../PuTTie/winfrip_rtl_registry.c
  ../PuTTie/winfrip_rtl_serialise.c
  ../PuTTie/winfrip_rtl_terminal.c
  ../PuTTie/winfrip_rtl_tree.c
  ../PuTTie/winfrip_storage.c
  ../PuTTie/winfrip_storage_adapter.c
  ../PuTTie/winfrip_storage_adapter_host_ca.c
  ../PuTTie/winfrip_storage_adapter_host_keys.c
  ../PuTTie/winfrip_storage_adapter_sessions.c
  ../PuTTie/winfrip_storage_backend_ephemeral.c
  ../PuTTie/winfrip_storage_backend_file.c
  ../PuTTie/winfrip_storage_backend_registry.c
  ../PuTTie/winfrip_storage_host_ca.c
  ../PuTTie/winfrip_storage_host_keys.c
  ../PuTTie/winfrip_storage_jump_list.c
  ../PuTTie/winfrip_storage_options.c
  ../PuTTie/winfrip_storage_priv.c
  ../PuTTie/winfrip_storage_privkey_list.c
  ../PuTTie/winfrip_storage_sessions.c
  ../PuTTie/winfrip_subr_putty_config.c)
endif()
# winfrip }}}
be_list(puttytel PuTTYtel SERIAL OTHERBACKENDS)
add_dependencies(puttytel generated_licence_h)
target_link_libraries(puttytel
  guiterminal guimisc eventloop otherbackends settings network utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2@master/libpcre2-16.a
  gdiplus.lib msimg32.lib
  bcrypt.lib
  ${platform_libraries})
if (MINGW)
  target_link_options(puttytel PRIVATE
    -Wl,-Bstatic -l:libwinpthread.a)
endif()
set_target_properties(puttytel PROPERTIES
  WIN32_EXECUTABLE ON
  LINK_FLAGS "${LFLAG_MANIFEST_NO}")
installed_program(puttytel)

add_executable(puttygen
  puttygen.c
  ${CMAKE_SOURCE_DIR}/stubs/no-timing.c
  noise.c
  no-jump-list.c
  ../PuTTie/no-jump-list-puttygen.c
  storage.c
  help.c
  ${CMAKE_SOURCE_DIR}/sshpubk.c
  ${CMAKE_SOURCE_DIR}/sshrand.c
  controls.c
  puttygen.rc)
add_dependencies(puttygen generated_licence_h)
target_link_libraries(puttygen
  keygen guimisc crypto utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2@master/libpcre2-16.a
  ${platform_libraries})
if (MINGW)
  target_link_options(puttygen PRIVATE
    -Wl,-Bstatic -l:libwinpthread.a)
endif()
set_target_properties(puttygen PROPERTIES
  WIN32_EXECUTABLE ON
  LINK_FLAGS "${LFLAG_MANIFEST_NO}")
installed_program(puttygen)

if(HAVE_CONPTY)
  add_executable(pterm
    window.c
    pterm.c
    help.c
    conpty.c
    ${CMAKE_SOURCE_DIR}/stubs/no-gss.c
    ${CMAKE_SOURCE_DIR}/stubs/no-ca-config.c
    ${CMAKE_SOURCE_DIR}/stubs/no-console.c
    ${CMAKE_SOURCE_DIR}/stubs/no-rand.c
    ${CMAKE_SOURCE_DIR}/proxy/nosshproxy.c
    pterm.rc)
  be_list(pterm pterm)
  add_dependencies(pterm generated_licence_h)
  target_link_libraries(pterm
    guiterminal guimisc eventloop settings network utils
    ${platform_libraries})
  if (MINGW)
    target_link_options(pterm PRIVATE
      -Wl,-Bstatic -l:libwinpthread.a)
  endif()
  set_target_properties(pterm PROPERTIES
    WIN32_EXECUTABLE ON
    LINK_FLAGS "${LFLAG_MANIFEST_NO}")
  installed_program(pterm)
else()
  message("ConPTY not available; cannot build Windows pterm")
endif()

add_executable(test_split_into_argv
  utils/split_into_argv.c)
target_compile_definitions(test_split_into_argv PRIVATE TEST)
target_link_libraries(test_split_into_argv utils ${platform_libraries})

add_executable(test_screenshot
  test_screenshot.c)
target_link_libraries(test_screenshot utils ${platform_libraries})
