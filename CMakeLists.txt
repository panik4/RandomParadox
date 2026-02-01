cmake_minimum_required(VERSION 3.15)

project(RandomParadox VERSION 1.0 LANGUAGES CXX)

# -----------------------------
# C++ Standard
# -----------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
# Compiler flags
# -----------------------------
if (MSVC)
    # Multi-threaded compilation
    add_compile_options(/MP)
    # Enable function-level linking
    add_compile_options(/Gy)
    # Warning level
    add_compile_options(/W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Optional: optimization for Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# -----------------------------
# Source files
# -----------------------------
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)
file(GLOB_RECURSE HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)
# -----------------------------
# AddressSanitizer (Clang/GCC only)
# -----------------------------
option(ENABLE_ASAN "Enable AddressSanitizer" ON)

if (ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    message(STATUS "Building with AddressSanitizer")

    add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -g
    )

    add_link_options(
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )

elseif (ENABLE_ASAN AND MSVC)
    message(WARNING "ASan requested but MSVC does NOT support it — use Clang on Windows.")
endif()


# -----------------------------
# Library target
# -----------------------------
add_library(RandomParadoxLib ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(RandomParadoxLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# -----------------------------
# External dependencies
# -----------------------------
# ArdaGen
add_subdirectory(${CMAKE_SOURCE_DIR}/external/ArdaSuite/ArdaGen)
add_dependencies(RandomParadoxLib ArdaGen)
target_link_libraries(RandomParadoxLib PUBLIC ArdaGen)

# ImGui (optional GUI support)
if (TARGET ImGui)
    target_link_libraries(RandomParadoxLib PUBLIC ImGui)
endif()

# -----------------------------
# Executable target
# -----------------------------
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE RandomParadoxLib)

# -----------------------------
# Print include directories (optional)
# -----------------------------
get_target_property(rp_inc_dirs RandomParadoxLib INCLUDE_DIRECTORIES)
message(STATUS "RandomParadox include directories: ${rp_inc_dirs}")
