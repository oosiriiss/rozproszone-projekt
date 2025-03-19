cmake_minimum_required(VERSION 3.10)

project(rozproszone-projekt)

add_executable(executable)
   

# Add sourc files here
target_sources(executable
   PRIVATE
   src/main.cpp
   src/logging.hpp
   src/network/server.cpp
   src/network/client.cpp
)











