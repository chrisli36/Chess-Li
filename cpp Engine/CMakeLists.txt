cmake_minimum_required(VERSION 3.10)
project(SFMLChessStarter)

set(CMAKE_CXX_STANDARD 17)

# Homebrew SFML path
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml")

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add executable
add_executable(chessli
    src/main.cpp
    src/board.cpp
    src/chess_ui.cpp
)

# Link SFML
target_link_libraries(chessli PRIVATE sfml-graphics sfml-window sfml-system)