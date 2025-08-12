cmake_minimum_required(VERSION 3.10)
project(SFMLChessStarter)

set(CMAKE_CXX_STANDARD 17)

# Optimization flags for maximum performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Maximum optimization for release builds
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
    # Enable link-time optimization
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    # Enable interprocedural optimization
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
else()
    # Debug build with some optimization
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g")
endif()

# Homebrew SFML path
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml@2")

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add executable
add_executable(chessli
    src/main.cpp
    src/board.cpp
    src/chess_ui.cpp
    src/engine.cpp
)

# Link SFML
target_link_libraries(chessli PRIVATE sfml-graphics sfml-window sfml-system)