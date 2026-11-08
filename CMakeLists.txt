cmake_minimum_required(VERSION 3.15)

project(RandomParadox VERSION 1.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(CTest)
endif()

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
    add_compile_options(/bigobj)
    add_compile_options(/guard:cf)
    add_link_options(/guard:cf)
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
    message(WARNING "ASan requested but MSVC does NOT support it � use Clang on Windows.")
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
add_executable(${PROJECT_NAME} main.cpp resources.rc)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources")
target_link_libraries(${PROJECT_NAME} PRIVATE RandomParadoxLib)
if (MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE "/STACK:33554432")
endif()

# -----------------------------
# Print include directories (optional)
# -----------------------------
get_target_property(rp_inc_dirs RandomParadoxLib INCLUDE_DIRECTORIES)
message(STATUS "RandomParadox include directories: ${rp_inc_dirs}")

if (BUILD_TESTING)
    add_executable(RandomParadoxStrategicRegionTests tests/StrategicRegionTests.cpp)
    target_link_libraries(RandomParadoxStrategicRegionTests PRIVATE RandomParadoxLib)
    add_test(NAME RandomParadoxStrategicRegionTests COMMAND RandomParadoxStrategicRegionTests)
    add_executable(RandomParadoxNavmeshExportTests tests/NavmeshExportTests.cpp)
    target_link_libraries(RandomParadoxNavmeshExportTests PRIVATE RandomParadoxLib)
    add_test(NAME RandomParadoxNavmeshExportTests COMMAND RandomParadoxNavmeshExportTests)
    set(navmesh_test_work "${CMAKE_CURRENT_BINARY_DIR}/work/NavmeshExportTests")
    file(MAKE_DIRECTORY "${navmesh_test_work}")
    set_tests_properties(RandomParadoxNavmeshExportTests PROPERTIES
        WORKING_DIRECTORY "${navmesh_test_work}" TIMEOUT 120)
endif()

if (BUILD_TESTING)
    add_executable(RandomParadoxCountryGenerationTests tests/CountryGenerationTests.cpp)
    target_link_libraries(RandomParadoxCountryGenerationTests PRIVATE RandomParadoxLib)
    add_test(NAME RandomParadoxCountryGenerationTests COMMAND RandomParadoxCountryGenerationTests)
endif()

if (BUILD_TESTING)
    add_executable(RandomParadoxBuildingPlacementTests tests/BuildingPlacementTests.cpp)
    target_link_libraries(RandomParadoxBuildingPlacementTests PRIVATE RandomParadoxLib)
    add_test(NAME RandomParadoxBuildingPlacementTests COMMAND RandomParadoxBuildingPlacementTests)
    set_tests_properties(RandomParadoxBuildingPlacementTests PROPERTIES TIMEOUT 60)
    set_tests_properties(
        RandomParadoxStrategicRegionTests
        RandomParadoxNavmeshExportTests
        RandomParadoxCountryGenerationTests
        RandomParadoxBuildingPlacementTests
        PROPERTIES LABELS "RandomParadox")
endif()
