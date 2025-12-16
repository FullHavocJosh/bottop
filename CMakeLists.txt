# SPDX-License-Identifier: Apache-2.0
#
# CMake configuration for bottop
# Linux-only AzerothCore bot monitor based on btop++
#

cmake_minimum_required(VERSION 3.25)

# Disable in-source builds since they would override the Makefile
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed")
endif()

project("bottop"
  DESCRIPTION "AzerothCore Bot Monitor - A Linux-only fork of btop for monitoring WoW bot servers"
  HOMEPAGE_URL "https://github.com/yourusername/bottop"
  LANGUAGES CXX
)

include(CheckIPOSupported)

# Make our Find<Package>.cmake files available (if any remain)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Verify we're on Linux
if(NOT LINUX)
  message(FATAL_ERROR "bottop only supports Linux. For other platforms, use the original btop++: https://github.com/aristocratos/btop")
endif()

option(BTOP_STATIC "Link bottop statically" OFF)

# Enable LTO in release builds by default
if(CMAKE_BUILD_TYPE MATCHES "Rel(ease|WithDebInfo)")
  option(BTOP_LTO "Enable LTO" ON)
else()
  option(BTOP_LTO "Enable LTO" OFF)
endif()

if(BTOP_STATIC)
  # Set this before calling find_package
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

add_executable(bottop src/main.cpp)
add_library(libbtop OBJECT
  src/btop.cpp
  src/btop_cli.cpp
  src/btop_config.cpp
  src/btop_draw.cpp
  src/btop_input.cpp
  src/btop_menu.cpp
  src/btop_shared.cpp
  src/btop_theme.cpp
  src/btop_tools.cpp
  # src/linux/btop_collect.cpp  # REMOVED: Stock btop system monitoring - bottop uses btop_collect_stub.cpp instead
  src/btop_collect_stub.cpp
)

target_link_libraries(bottop libbtop)

if(BTOP_LTO)
  check_ipo_supported()
  set_target_properties(bottop PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  set_target_properties(libbtop PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Generate build info
execute_process(
  COMMAND "git" "rev-parse" "--short" "HEAD"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
set(CONFIGURE_COMMAND "cmake -DBTOP_STATIC=${BTOP_STATIC}")
get_filename_component(CXX_COMPILER_BASENAME "${CMAKE_CXX_COMPILER}" NAME)
set(COMPILER "${CXX_COMPILER_BASENAME}")
set(COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY IMMEDIATE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)

# Verify C++23 features
check_cxx_source_compiles(
  "
  #include <version>
  #if __cpp_lib_optional < 202110L
  #error \"Missing __cpp_lib_optional < 202110L\"
  #endif
  int main() { return 0; }
  "
  HAS_CXX_OPTIONAL_MONADS
)
if(NOT HAS_CXX_OPTIONAL_MONADS)
  message(FATAL_ERROR "The compiler doesn't support std::optional::and_then")
endif()

check_cxx_symbol_exists("__cpp_lib_expected" "version" HAS_CXX_EXPECTED)
if(NOT HAS_CXX_EXPECTED)
  message(FATAL_ERROR "The compiler doesn't support std::expected")
endif()

check_cxx_symbol_exists("__cpp_lib_ranges" "version" HAS_CXX_RANGES)
if(NOT HAS_CXX_RANGES)
  message(FATAL_ERROR "The compiler doesn't support std::ranges")
endif()

check_cxx_symbol_exists("__cpp_lib_ranges_to_container" "version" HAS_CXX_RANGES_TO_CONTAINER)
if(NOT HAS_CXX_RANGES_TO_CONTAINER)
  message(FATAL_ERROR "The compiler doesn't support std::ranges::to")
endif()

check_cxx_symbol_exists("__cpp_lib_string_contains" "version" HAS_CXX_STRING_CONTAINS)
if(NOT HAS_CXX_STRING_CONTAINS)
  message(FATAL_ERROR "The compiler doesn't support std::string::contains")
endif()

target_compile_options(libbtop PUBLIC -Wall -Wextra -Wpedantic)

include(CheckCXXCompilerFlag)

target_compile_options(libbtop PUBLIC -fstack-clash-protection)
check_cxx_compiler_flag(-fstack-protector HAS_FSTACK_PROTECTOR)
if(HAS_FSTACK_PROTECTOR)
  target_compile_options(libbtop PUBLIC -fstack-protector)
endif()
check_cxx_compiler_flag(-fcf-protection HAS_FCF_PROTECTION)
if(HAS_FCF_PROTECTION)
  target_compile_options(libbtop PUBLIC -fcf-protection)
endif()

target_compile_definitions(libbtop PUBLIC
  FMT_HEADER_ONLY
  _FILE_OFFSET_BITS=64
  $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG>
)

target_include_directories(libbtop SYSTEM PUBLIC include)

# Enable pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libbtop Threads::Threads)

# Find libssh2 for AzerothCore monitoring
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH2 libssh2)
if(LIBSSH2_FOUND)
  target_sources(libbtop PRIVATE src/btop_azerothcore.cpp)
  target_compile_definitions(libbtop PUBLIC AZEROTHCORE_SUPPORT)
  target_include_directories(libbtop PRIVATE ${LIBSSH2_INCLUDE_DIRS})
  target_link_libraries(libbtop ${LIBSSH2_LIBRARIES})
  message(STATUS "AzerothCore Monitor support: ENABLED")
else()
  message(WARNING "AzerothCore Monitor support: DISABLED (libssh2 not found)")
  message(WARNING "Install libssh2-dev (Debian/Ubuntu) or libssh2-devel (Fedora/RHEL) to enable AzerothCore monitoring")
endif()

if(BTOP_STATIC)
  target_compile_definitions(libbtop PUBLIC STATIC_BUILD)
  target_link_options(libbtop PUBLIC -static LINKER:--fatal-warnings)
endif()

# Check if lowdown is installed for man page generation
find_program(LOWDOWN_EXECUTABLE lowdown)

if(LOWDOWN_EXECUTABLE)
  # Custom target to compile Markdown to man page using lowdown
  add_custom_command(
    OUTPUT bottop.1
    COMMAND lowdown -s -T man -o bottop.1 ${CMAKE_CURRENT_SOURCE_DIR}/manpage.md
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/manpage.md
    VERBATIM
  )
  add_custom_target(generate_manpage ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bottop.1)
  # Install the man page
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bottop.1" DESTINATION "share/man/man1")
else()
  message(STATUS "Command 'lowdown' not found: skipping man page generation")
endif()

install(TARGETS bottop RUNTIME)
install(FILES "btop.desktop" DESTINATION "share/applications" RENAME "bottop.desktop")
install(FILES "Img/icon.png" DESTINATION "share/icons/hicolor/48x48/apps" RENAME "bottop.png")
install(FILES "Img/icon.svg" DESTINATION "share/icons/hicolor/scalable/apps" RENAME "bottop.svg")
install(DIRECTORY "themes" DESTINATION "share/bottop")

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "bottop Configuration Summary")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: Linux only")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Static linking: ${BTOP_STATIC}")
message(STATUS "LTO enabled: ${BTOP_LTO}")
if(LIBSSH2_FOUND)
  message(STATUS "AzerothCore support: YES")
else()
  message(STATUS "AzerothCore support: NO (install libssh2)")
endif()
message(STATUS "========================================")
message(STATUS "")
