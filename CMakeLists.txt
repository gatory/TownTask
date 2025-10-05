cmake_minimum_required(VERSION 3.15)
project(TownTask)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "D:/raylib/raylib-5.0_win64_msvc16")
set(raylib_DIR "D:/raylib/raylib-5.0_win64_msvc16/lib/cmake/raylib")

# Find OpenCV
set(OpenCV_DIR "D:/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/raygui_impl.cpp
    src/Core/World.cpp
    src/Data/DataManager.cpp
    src/Systems/AnimationSystem.cpp
    src/Systems/MovementSystem.cpp
    src/Systems/CollisionSystem.cpp
    src/Systems/InteractionSystem.cpp
    src/Systems/RenderSystem.cpp
    src/Systems/GUISystem.cpp
    src/Systems/LibraryUISystem.cpp
    src/Systems/TodoUISystem.cpp
    src/Systems/FaceDetectionSystem.cpp
    src/Game/Game.cpp
    src/Game/SceneManager.cpp
    src/Game/GameState.cpp
)

# Header files
set(HEADERS
    include/Core/Constants.h
    include/Core/Types.h
    include/Core/Components.h
    include/Core/World.h
    include/Core/EntityView.h
    include/Data/DataManager.h
    include/Systems/AnimationSystem.h
    include/Systems/MovementSystem.h
    include/Systems/CollisionSystem.h
    include/Systems/InteractionSystem.h
    include/Systems/RenderSystem.h
    include/Systems/GUISystem.h
    include/Systems/LibraryUISystem.h
    include/Systems/TodoUISystem.h
    include/Systems/FaceDetectionSystem.h
    include/Resources/TextureManager.h
    include/Game/Game.h
    include/Game/SceneManager.h
    include/Game/GameState.h
)

include_directories("D:/raylib/raylib-5.0_win64_msvc16/include")
link_directories("D:/raylib/raylib-5.0_win64_msvc16/lib")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
if (WIN32)
    # raylib (static/dll) may require winmm for multimedia timer functions (timeBeginPeriod/timeEndPeriod)
    target_link_libraries(${PROJECT_NAME} raylib winmm ${OpenCV_LIBS})
else()
    target_link_libraries(${PROJECT_NAME} raylib ${OpenCV_LIBS})
endif()

# Copy assets folder to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Create data directory in build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)
