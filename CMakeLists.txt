cmake_minimum_required(VERSION 3.25)

project(cayene_decoder
    VERSION 0.1.0
    DESCRIPTION "Cayene Decoder Library"
    LANGUAGES CXX
)

# ============================================================================
# Global Settings
# ============================================================================
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use Clang as the compiler
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "This project is designed for Clang. Current compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# ============================================================================
# Options
# ============================================================================
option(CAYENE_BUILD_TESTS "Build tests" ON)
option(CAYENE_BUILD_EXAMPLES "Build examples" ON)
option(CAYENE_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(CAYENE_ENABLE_SANITIZERS "Enable sanitizers in debug mode" OFF)

# ============================================================================
# Compiler Warnings (C++ Core Guidelines compliant)
# ============================================================================
add_library(cayene_warnings INTERFACE)

if(CAYENE_ENABLE_WARNINGS)
    target_compile_options(cayene_warnings INTERFACE
        $<$<CXX_COMPILER_ID:Clang>:
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        >
    )
endif()

# ============================================================================
# Sanitizers (Debug only)
# ============================================================================
add_library(cayene_sanitizers INTERFACE)

if(CAYENE_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(cayene_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(cayene_sanitizers INTERFACE
        -fsanitize=address,undefined
    )
endif()

# ============================================================================
# Dependencies
# ============================================================================
include(FetchContent)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Google Test (only if tests are enabled)
if(CAYENE_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# ============================================================================
# Library
# ============================================================================
add_library(cayene_decoder
    src/decoder.cpp
)

target_include_directories(cayene_decoder
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cayene_decoder
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        $<BUILD_INTERFACE:cayene_warnings>
        $<BUILD_INTERFACE:cayene_sanitizers>
)

# Alias for uniform usage
add_library(cayene::decoder ALIAS cayene_decoder)

# ============================================================================
# IntelliSense Support - Auto-update compile_commands.json symlink
# ============================================================================
add_custom_target(update_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Updating compile_commands.json symlink for IntelliSense"
    VERBATIM
)

# ============================================================================
# Tests
# ============================================================================
if(CAYENE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# Examples
# ============================================================================
if(CAYENE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Install (optional - only install the library itself)
# ============================================================================
include(GNUInstallDirs)

install(TARGETS cayene_decoder
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
