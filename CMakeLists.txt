cmake_minimum_required(VERSION 3.10)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

project(rozproszone-projekt)

add_executable(executable)
   

# Add sourc files here
target_sources(executable
   PRIVATE
   src/main.cpp
   src/logging.hpp
   src/network/socket.cpp
   src/network/server.cpp
   src/network/client.cpp
)











