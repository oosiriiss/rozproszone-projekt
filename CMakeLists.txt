cmake_minimum_required(VERSION 3.10)

project(rozproszone-projekt)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)



include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

  
add_executable(executable)

# Add sourc files here
target_sources(executable
   PRIVATE
   src/main.cpp
   src/Application.cpp
   src/logging.hpp
   src/game/player.cpp
   src/network/socket.cpp
   src/network/server.cpp
   src/network/client.cpp
   src/network/packet.cpp
)



target_compile_features(executable PRIVATE cxx_std_17)
target_link_libraries(executable PRIVATE SFML::Graphics)
