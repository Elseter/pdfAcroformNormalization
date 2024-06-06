cmake_minimum_required(VERSION 3.28)
project(untitled)

set(PODOFO_DIR /opt/homebrew/Cellar/podofo/0.10.3) # Set up the library path

include_directories(${PODOFO_DIR}/include/) # include the lib
link_directories(${PODOFO_DIR}/lib/)


set(CMAKE_CXX_STANDARD 17)
add_executable(untitled main.cpp)
target_link_libraries(untitled podofo)

