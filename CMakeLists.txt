# CMakeList.txt : CMake project for usb-logger, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.16)

project("usb-logger" LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Threads: find_package gives us Threads::Threads which maps to -pthread
# (compile + link flag), not just -lpthread (link only).
# Required by std::thread in app.cpp on all platforms.
# ---------------------------------------------------------------------------
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# GCC < 9 ships std::filesystem in a separate libstdc++fs.
# GCC >= 9 includes it in libstdc++ automatically.
# ---------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    set(STDCXXFS_LIB stdc++fs)
else()
    set(STDCXXFS_LIB "")
endif()

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(usb-logger
  src/main.cpp
  src/app.cpp
  src/config_loader.cpp
  src/file_utils.cpp
  src/line_assembler.cpp
  src/line_logger.cpp
  src/serial_port.cpp
  src/terminal_input.cpp
  src/csv_writer.cpp
)

# Ensure VS/remote sync includes non-code assets (scripts/config/systemd).
# This does not affect compilation.
target_sources(usb-logger PRIVATE
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
    config/app.toml
    scripts/install.sh
    scripts/uninstall.sh
    scripts/make_package.sh
    scripts/remote-reconfigure.sh
    #systemd/usb-logger.service
)

target_include_directories(usb-logger PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(usb-logger PRIVATE cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(usb-logger PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(usb-logger PRIVATE
  ${STDCXXFS_LIB}
  Threads::Threads
)

install(TARGETS usb-logger RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/app.toml
        DESTINATION share/usb-logger RENAME app.toml.example)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(CTest)
if (BUILD_TESTING)
  add_executable(usb-logger-tests
    tests/test_main.cpp
    tests/test_line_assembler.cpp
    tests/test_config_loader.cpp

    src/line_assembler.cpp
    src/config_loader.cpp
    src/file_utils.cpp
  )

  target_include_directories(usb-logger-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_compile_features(usb-logger-tests PRIVATE cxx_std_20)

  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(usb-logger-tests PRIVATE -Wall -Wextra -Wpedantic)
  endif()

  # Tests also use std::filesystem (file_utils, config_loader) — same libs needed.
  target_link_libraries(usb-logger-tests PRIVATE
    ${STDCXXFS_LIB}
    Threads::Threads
  )

  add_test(NAME usb-logger.unit COMMAND usb-logger-tests)
endif()
