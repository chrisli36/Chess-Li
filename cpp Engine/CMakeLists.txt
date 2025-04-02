cmake_minimum_required(VERSION 3.10)
project(ChessLi)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(chessli
    src/main.cpp
    src/board.cpp
    src/piece.cpp
)