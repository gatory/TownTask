cmake_minimum_required(VERSION 3.16)
project(TaskTown VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Performance optimization options
option(ENABLE_PERFORMANCE_OPTIMIZATIONS "Enable aggressive performance optimizations" ON)
option(ENABLE_PROFILING "Enable performance profiling" OFF)

# Find packages
find_package(raylib QUIET)
if(NOT raylib_FOUND)
    # If raylib is not found, we'll use FetchContent to download it
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Find nlohmann/json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Find Google Test for unit testing
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Create executable
add_executable(TaskTown ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(TaskTown raylib)
if(nlohmann_json_FOUND)
    target_link_libraries(TaskTown nlohmann_json::nlohmann_json)
else()
    target_link_libraries(TaskTown nlohmann_json)
endif()

# Include directories
target_include_directories(TaskTown PRIVATE src/)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Platform-specific configurations
if(WIN32)
    set_target_properties(TaskTown PROPERTIES WIN32_EXECUTABLE TRUE)
    # Add Windows-specific flags
    target_compile_definitions(TaskTown PRIVATE PLATFORM_DESKTOP)
elseif(APPLE)
    set_target_properties(TaskTown PROPERTIES MACOSX_BUNDLE TRUE)
    # Add macOS-specific flags
    target_compile_definitions(TaskTown PRIVATE PLATFORM_DESKTOP)
elseif(UNIX)
    # Linux-specific flags
    target_compile_definitions(TaskTown PRIVATE PLATFORM_DESKTOP)
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(TaskTown PRIVATE /W4)
    
    # MSVC performance optimizations
    if(ENABLE_PERFORMANCE_OPTIMIZATIONS AND CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(TaskTown PRIVATE /O2 /Ob2 /GL /fp:fast)
        target_link_options(TaskTown PRIVATE /LTCG /OPT:REF /OPT:ICF)
    endif()
else()
    target_compile_options(TaskTown PRIVATE -Wall -Wextra -Wpedantic)
    
    # GCC/Clang performance optimizations
    if(ENABLE_PERFORMANCE_OPTIMIZATIONS AND CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(TaskTown PRIVATE -O3 -march=native -mtune=native -flto -ffast-math)
        target_link_options(TaskTown PRIVATE -flto)
        
        # Additional optimizations for GCC
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(TaskTown PRIVATE -fuse-linker-plugin)
            target_link_options(TaskTown PRIVATE -fuse-linker-plugin)
        endif()
    endif()
endif()

# Profiling support
if(ENABLE_PROFILING)
    target_compile_definitions(TaskTown PRIVATE ENABLE_PROFILING=1)
    if(NOT MSVC)
        target_compile_options(TaskTown PRIVATE -pg)
        target_link_options(TaskTown PRIVATE -pg)
    endif()
endif()

# Debug/Release configurations
target_compile_definitions(TaskTown PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Build tests if in Debug mode or if explicitly requested
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    # Test source files
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
    
    # Create test executable
    add_executable(TaskTownTests ${TEST_SOURCES} ${SOURCES})
    
    # Remove main.cpp from test build to avoid multiple main functions
    list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    get_target_property(MAIN_SOURCES TaskTown SOURCES)
    list(REMOVE_ITEM MAIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    
    # Rebuild test executable with correct sources
    set_target_properties(TaskTownTests PROPERTIES SOURCES "")
    target_sources(TaskTownTests PRIVATE ${TEST_SOURCES} ${MAIN_SOURCES})
    
    # Link test libraries
    target_link_libraries(TaskTownTests 
        gtest 
        gtest_main 
        raylib
    )
    
    if(nlohmann_json_FOUND)
        target_link_libraries(TaskTownTests nlohmann_json::nlohmann_json)
    else()
        target_link_libraries(TaskTownTests nlohmann_json)
    endif()
    
    # Include directories for tests
    target_include_directories(TaskTownTests PRIVATE src/)
    
    # Add test to CTest
    add_test(NAME TaskTownUnitTests COMMAND TaskTownTests)
endif()