cmake_minimum_required(VERSION 3.7)
project(putty LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(cmake/setup.cmake)

# Scan the docs directory first, so that when we start calling
# installed_program(), we'll know if we have man pages available
add_subdirectory(doc)

add_compile_definitions(HAVE_CMAKE_H)

include_directories(terminal)

add_library(utils STATIC
  ${GENERATED_COMMIT_C})
add_dependencies(utils cmake_commit_c)
add_subdirectory(utils)
add_subdirectory(stubs)

add_library(logging OBJECT
  logging.c utils/logeventf.c)

add_library(eventloop STATIC
  callback.c timing.c)

add_library(console STATIC
  clicons.c console.c)

add_library(settings STATIC
  cmdline.c settings.c)

add_library(crypto STATIC
  proxy/cproxy.c proxy/sshproxy.c)
add_subdirectory(crypto)

add_library(network STATIC
  errsock.c x11disp.c
  $<TARGET_OBJECTS:logging>
  proxy/proxy.c
  proxy/http.c
  proxy/socks4.c
  proxy/socks5.c
  proxy/telnet.c
  proxy/local.c
  proxy/interactor.c)

add_library(keygen STATIC
  import.c)
add_subdirectory(keygen)

add_library(agent STATIC
  sshpubk.c pageant.c aqsync.c)

add_library(guiterminal STATIC
  terminal/terminal.c terminal/bidi.c
  ldisc.c terminal/lineedit.c config.c dialog.c
  $<TARGET_OBJECTS:logging>)

add_library(noterminal STATIC
  stubs/no-term.c ldisc.c)

add_library(all-backends OBJECT
  pinger.c)

add_library(sftpclient STATIC
  psftpcommon.c)
add_subdirectory(ssh)

add_library(otherbackends STATIC
  $<TARGET_OBJECTS:all-backends>
  $<TARGET_OBJECTS:logging>)
add_subdirectory(otherbackends)

add_executable(testcrypt
  test/testcrypt.c sshpubk.c ssh/crc-attack-detector.c)
target_link_libraries(testcrypt
  keygen crypto utils ${platform_libraries})
# {{{ winfrip
if (MINGW)
set_property(TARGET testcrypt APPEND PROPERTY SOURCES
  PuTTie/winfrip_rtl.c)
set_property(TARGET testcrypt APPEND_STRING PROPERTY COMPILE_FLAGS
  " -DWINFRIP_RTL_NO_PCRE2")
endif()
set_property(TARGET testcrypt APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

add_executable(test_host_strfoo
  utils/host_strchr_internal.c)
target_compile_definitions(test_host_strfoo PRIVATE TEST)
target_link_libraries(test_host_strfoo utils ${platform_libraries})

add_executable(test_decode_utf8
  utils/decode_utf8.c)
target_compile_definitions(test_decode_utf8 PRIVATE TEST)
target_link_libraries(test_decode_utf8 utils ${platform_libraries})

add_executable(test_unicode_norm
  utils/unicode-norm.c)
target_compile_definitions(test_unicode_norm PRIVATE TEST)
target_link_libraries(test_unicode_norm utils ${platform_libraries})
# {{{ winfrip
if (MINGW)
set_property(TARGET test_unicode_norm APPEND PROPERTY SOURCES
  PuTTie/winfrip_rtl.c)
set_property(TARGET test_unicode_norm APPEND PROPERTY COMPILE_FLAGS
  "-DWINFRIP_RTL_NO_PCRE2")
endif()
# winfrip }}}

add_executable(test_tree234
  utils/tree234.c)
target_compile_definitions(test_tree234 PRIVATE TEST)
target_link_libraries(test_tree234 utils ${platform_libraries})

add_executable(test_wildcard
  utils/wildcard.c)
target_compile_definitions(test_wildcard PRIVATE TEST)
target_link_libraries(test_wildcard utils ${platform_libraries})

add_executable(test_cert_expr
  utils/cert-expr.c)
target_compile_definitions(test_cert_expr PRIVATE TEST)
target_link_libraries(test_cert_expr utils ${platform_libraries})

add_executable(bidi_gettype
  terminal/bidi_gettype.c)
target_link_libraries(bidi_gettype guiterminal utils ${platform_libraries})

add_executable(bidi_test
  terminal/bidi_test.c)
target_link_libraries(bidi_test guiterminal utils ${platform_libraries})
# {{{ winfrip
if (MINGW)
set_property(TARGET bidi_test APPEND PROPERTY SOURCES
  PuTTie/winfrip_rtl.c)
set_property(TARGET bidi_test APPEND PROPERTY COMPILE_FLAGS
  "-DWINFRIP_RTL_NO_PCRE2")
endif()
# winfrip }}}

# {{{ winfrip
if (MINGW)
set_source_files_properties(PuTTie/winfrip_rtl.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")

set_source_files_properties(PuTTie/winfrip_rtl_debug.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
if ("${WINFRIP_DEBUG_NOCONSOLE}" STREQUAL "1")
  set_property(SOURCE PuTTie/winfrip_rtl_debug.c APPEND_STRING PROPERTY COMPILE_FLAGS
    " -DWINFRIP_DEBUG_NOCONSOLE")
endif()

set_source_files_properties(PuTTie/winfrip_rtl_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_load.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_pcre2.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_registry.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_save.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_tree.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_rtl_windows.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_adapter.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_adapter_host_ca.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_adapter_host_keys.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_adapter_sessions.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_backend_ephemeral.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_backend_file.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_backend_registry.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_host_ca.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_host_keys.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_jump_list.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_options.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_priv.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_privkey_list.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")
set_source_files_properties(PuTTie/winfrip_storage_sessions.c PROPERTIES COMPILE_FLAGS "-Wall -Wextra")

if ("${DEFAULT_STORAGE_BACKEND}" STREQUAL "")
  set(DEFAULT_STORAGE_BACKEND "WFS_BACKEND_REGISTRY")
endif()
set_property(SOURCE PuTTie/winfrip_storage.c APPEND_STRING PROPERTY COMPILE_FLAGS " -DWFS_DEFAULT_STORAGE_BACKEND=${DEFAULT_STORAGE_BACKEND}")

if ("${BUILD_SSH_VERSION}" STREQUAL "")
  set(BUILD_SSH_VERSION "-Unidentified-Local-Build")
endif()
if ("${BUILD_TEXT_VERSION}" STREQUAL "")
  set(BUILD_TEXT_VERSION "Unidentified build")
endif()
set_property(SOURCE utils/version.c APPEND_STRING PROPERTY COMPILE_FLAGS " -DSSHVER=\"\\\"${BUILD_SSH_VERSION}\\\"\" -DTEXTVER=\"\\\"${BUILD_TEXT_VERSION}\\\"\"")

endif()
# winfrip }}}

add_executable(plink
  ${platform}/plink.c
  stubs/no-lineedit.c)
# {{{ winfrip
if (MINGW)
set_property(TARGET plink APPEND PROPERTY SOURCES
  PuTTie/no-jump-list.c
  PuTTie/winfrip_rtl.c
  PuTTie/winfrip_rtl_debug.c
  PuTTie/winfrip_rtl_file.c
  PuTTie/winfrip_rtl_load.c
  PuTTie/winfrip_rtl_pcre2.c
  PuTTie/winfrip_rtl_registry.c
  PuTTie/winfrip_rtl_save.c
  PuTTie/winfrip_rtl_tree.c
  PuTTie/winfrip_rtl_windows.c
  PuTTie/winfrip_storage.c
  PuTTie/winfrip_storage_adapter.c
  PuTTie/winfrip_storage_adapter_host_ca.c
  PuTTie/winfrip_storage_adapter_host_keys.c
  PuTTie/winfrip_storage_adapter_sessions.c
  PuTTie/winfrip_storage_backend_ephemeral.c
  PuTTie/winfrip_storage_backend_file.c
  PuTTie/winfrip_storage_backend_registry.c
  PuTTie/winfrip_storage_host_ca.c
  PuTTie/winfrip_storage_host_keys.c
  PuTTie/winfrip_storage_jump_list.c
  PuTTie/winfrip_storage_options.c
  PuTTie/winfrip_storage_priv.c
  PuTTie/winfrip_storage_privkey_list.c
  PuTTie/winfrip_storage_sessions.c)
endif()
# winfrip }}}
# Note: if we ever port Plink to a platform where we can't implement a
# serial backend, this be_list command will need to become platform-
# dependent, so that it only sets the SERIAL option on platforms where
# that backend exists. For the moment, though, we have serial port
# backends for both our platforms, so we can do this unconditionally.
be_list(plink Plink SSH SERIAL OTHERBACKENDS)
target_link_libraries(plink
  eventloop noterminal console sshclient otherbackends settings network crypto
  utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2/libpcre2-16.a
  ${platform_libraries})
installed_program(plink)
# {{{ winfrip
set_property(TARGET plink APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

add_executable(pscp
  pscp.c)
# {{{ winfrip
if (MINGW)
set_property(TARGET pscp APPEND PROPERTY SOURCES
  PuTTie/no-jump-list.c
  PuTTie/winfrip_rtl.c
  PuTTie/winfrip_rtl_debug.c
  PuTTie/winfrip_rtl_file.c
  PuTTie/winfrip_rtl_load.c
  PuTTie/winfrip_rtl_pcre2.c
  PuTTie/winfrip_rtl_registry.c
  PuTTie/winfrip_rtl_save.c
  PuTTie/winfrip_rtl_tree.c
  PuTTie/winfrip_rtl_windows.c
  PuTTie/winfrip_storage.c
  PuTTie/winfrip_storage_adapter.c
  PuTTie/winfrip_storage_adapter_host_ca.c
  PuTTie/winfrip_storage_adapter_host_keys.c
  PuTTie/winfrip_storage_adapter_sessions.c
  PuTTie/winfrip_storage_backend_ephemeral.c
  PuTTie/winfrip_storage_backend_file.c
  PuTTie/winfrip_storage_backend_registry.c
  PuTTie/winfrip_storage_host_ca.c
  PuTTie/winfrip_storage_host_keys.c
  PuTTie/winfrip_storage_jump_list.c
  PuTTie/winfrip_storage_options.c
  PuTTie/winfrip_storage_priv.c
  PuTTie/winfrip_storage_privkey_list.c
  PuTTie/winfrip_storage_sessions.c)
endif()
# winfrip }}}
be_list(pscp PSCP SSH)
target_link_libraries(pscp
  sftpclient eventloop console sshclient settings network crypto utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2/libpcre2-16.a
  ${platform_libraries})
installed_program(pscp)
# {{{ winfrip
set_property(TARGET pscp APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

add_executable(psftp
  psftp.c)
# {{{ winfrip
if (MINGW)
set_property(TARGET psftp APPEND PROPERTY SOURCES
  PuTTie/no-jump-list.c
  PuTTie/winfrip_rtl.c
  PuTTie/winfrip_rtl_debug.c
  PuTTie/winfrip_rtl_file.c
  PuTTie/winfrip_rtl_load.c
  PuTTie/winfrip_rtl_pcre2.c
  PuTTie/winfrip_rtl_registry.c
  PuTTie/winfrip_rtl_save.c
  PuTTie/winfrip_rtl_tree.c
  PuTTie/winfrip_rtl_windows.c
  PuTTie/winfrip_storage.c
  PuTTie/winfrip_storage_adapter.c
  PuTTie/winfrip_storage_adapter_host_ca.c
  PuTTie/winfrip_storage_adapter_host_keys.c
  PuTTie/winfrip_storage_adapter_sessions.c
  PuTTie/winfrip_storage_backend_ephemeral.c
  PuTTie/winfrip_storage_backend_file.c
  PuTTie/winfrip_storage_backend_registry.c
  PuTTie/winfrip_storage_host_ca.c
  PuTTie/winfrip_storage_host_keys.c
  PuTTie/winfrip_storage_jump_list.c
  PuTTie/winfrip_storage_options.c
  PuTTie/winfrip_storage_priv.c
  PuTTie/winfrip_storage_privkey_list.c
  PuTTie/winfrip_storage_sessions.c)
endif()
# winfrip }}}
be_list(psftp PSFTP SSH)
target_link_libraries(psftp
  sftpclient eventloop console sshclient settings network crypto utils
  ${CMAKE_SOURCE_DIR}/PuTTie/pcre2/libpcre2-16.a
  ${platform_libraries})
installed_program(psftp)
# {{{ winfrip
set_property(TARGET psftp APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

add_executable(psocks
  ${platform}/psocks.c
  psocks.c
  stubs/no-rand.c
  proxy/nocproxy.c
  proxy/nosshproxy.c
  ssh/portfwd.c)
# {{{ winfrip
if (MINGW)
set_property(TARGET psocks APPEND PROPERTY SOURCES
  PuTTie/winfrip_rtl.c)
set_property(TARGET psocks APPEND PROPERTY COMPILE_FLAGS
  "-DWINFRIP_RTL_NO_PCRE2")
endif()
# winfrip }}}
target_link_libraries(psocks
  eventloop console network utils
  ${platform_libraries})
# {{{ winfrip
set_property(TARGET psocks APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

add_executable(test_conf
  test/test_conf.c
  stubs/no-agent.c
  stubs/no-callback.c
  stubs/no-gss.c
  stubs/no-ldisc.c
  stubs/no-network.c
  stubs/no-timing.c
  proxy/noproxy.c # FIXME: move this to stubs
)
be_list(test_conf TestConf SSH SERIAL OTHERBACKENDS)
target_link_libraries(test_conf sshclient otherbackends settings network crypto utils ${platform_libraries})
# {{{ winfrip
set_property(TARGET test_conf APPEND PROPERTY SOURCES
  PuTTie/stubs-cachepasswords.c)
# winfrip }}}

foreach(subdir ${platform} ${extra_dirs})
  add_subdirectory(${subdir})
endforeach()

# {{{ winfrip
if (MINGW)
  add_subdirectory(PuTTie)
endif()
# winfrip }}}

configure_file(cmake/cmake.h.in ${GENERATED_SOURCES_DIR}/cmake.h)

# {{{ winfrip
#
# vim:expandtab sw=4 ts=4
#
# winfrip }}}
