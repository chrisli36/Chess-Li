cmake_minimum_required(VERSION 3.10)
project(SFMLChessStarter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Find SFML 3.0
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# Add executable
add_executable(chessli
    src/main.cpp
    src/board.cpp
    src/chess_ui.cpp
    src/engine.cpp
)

# Set include directories for SFML 3.0
target_include_directories(chessli PRIVATE 
    /opt/homebrew/Cellar/sfml/3.0.1/include
    ${SFML_INCLUDE_DIRS}
)

# Link SFML using targets (SFML 3.0 approach)
target_link_libraries(chessli PRIVATE SFML::Graphics SFML::Window SFML::System)