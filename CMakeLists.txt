cmake_minimum_required(VERSION 3.10)

project(rozproszone-projekt)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/lib")



add_executable(executable)
   
# Add sourc files here
target_sources(executable
   PRIVATE
   src/main.cpp
)











