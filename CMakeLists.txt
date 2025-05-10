cmake_minimum_required(VERSION 3.10)

project(rozproszone-projekt)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)



set(SOURCES 
   src/main.cpp
   src/AssetManager.cpp
   src/Application.cpp
   src/Scene.cpp
   src/game/Player.cpp
   src/game/Enemy.cpp
   src/game/Level.cpp
   src/game/Fireball.cpp
   src/game/Base.cpp
   src/game/HealthBar.cpp
   src/network/socket.cpp
   src/network/server.cpp
   src/network/client.cpp
   src/network/packet.cpp
   src/ui/ui.cpp
)

 
# DEbug build options
add_executable(executable-debug ${SOURCES})
target_compile_definitions(executable-debug PRIVATE DEBUG_BUILD)
target_compile_options(executable-debug PRIVATE -g -O0)

# Release build specific fileds
add_executable(executable-release ${SOURCES})
target_compile_definitions(executable-release PRIVATE RELEASE_BUILD)
target_compile_options(executable-debug PRIVATE -O3)

# Common options
target_compile_features(executable-debug PRIVATE cxx_std_23)
target_compile_features(executable-release PRIVATE cxx_std_23)
target_link_libraries(executable-debug PRIVATE SFML::Graphics)
target_link_libraries(executable-release PRIVATE SFML::Graphics)
