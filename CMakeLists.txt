# Minimum CMake version
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(RandomParadox VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# For Release build (force optimizations)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2")
# Enable Link Time Optimization (LTO)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Enable MSVC-specific optimizations (if using MSVC compiler)
if(MSVC)
    add_compile_options(/MP)  # Enables multi-threaded compilation
    add_compile_options(/GL-)  # Disable whole program optimization
    add_compile_options(/Gy)  # Enable function-level linking
endif()

if(MSVC)
    add_compile_options(/std:c++latest)
endif()
# Add the include and source directories for RandomParadox
file(GLOB_RECURSE SOURCE_FILES source/*.cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h include/*.hpp)

# Add the library target
add_library(RandomParadoxLib ${SOURCE_FILES} ${HEADER_FILES})
# Add the ArdaGen subdirectory first
add_subdirectory(${CMAKE_SOURCE_DIR}/external/ArdaSuite/ArdaGen)
# Query the include directories for ArdaGen to ensure they are properly set
get_target_property(inc_dirs ArdaGen INCLUDE_DIRECTORIES)
message(STATUS "ArdaGen include directories: ${inc_dirs}")


# Ensure that RandomParadox depends on ArdaGen so that the libraries are built first
add_dependencies(RandomParadoxLib ArdaGen)

# Link the ArdaGen library to RandomParadox
target_link_libraries(RandomParadoxLib PUBLIC ArdaGen)
# Link ImGui to RandomParadoxLib
target_link_libraries(RandomParadoxLib PUBLIC ImGui)
target_include_directories(RandomParadoxLib PUBLIC 
${CMAKE_SOURCE_DIR}/include
)

get_target_property(rp_inc_dirs RandomParadoxLib INCLUDE_DIRECTORIES)
message(STATUS "RandomParadox include directories: ${rp_inc_dirs}")

# Executable target (FastWorldGen) ------------------------------------
add_executable(${PROJECT_NAME} main.cpp)
# Link executable with the library
target_link_libraries(${PROJECT_NAME} PUBLIC RandomParadoxLib)

   