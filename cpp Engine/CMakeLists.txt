cmake_minimum_required(VERSION 3.16)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

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

# Fetch Crow (HTTP/WebSocket framework) - Not needed for simple server

# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG master
)
FetchContent_MakeAvailable(nlohmann_json)

# Homebrew SFML path (for original chess app)
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml@2")

# Find SFML for the original chess application
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

# Add chess engine library (without chess_ui for now)
add_library(chess_engine
    src/board.cpp
    src/engine.cpp
    src/adapter.cpp
)

# Add original chess executable (if SFML is available)
# Note: This requires chess_ui.cpp which has SFML dependencies
# if(SFML_FOUND)
#     add_executable(chessli
#         src/main.cpp
#     )
#     target_link_libraries(chessli PRIVATE chess_engine sfml-graphics sfml-window sfml-system)
# endif()

# Add HTTP API server executable
add_executable(server
    src/simple_server_fixed.cpp
)

# Link dependencies for the server
target_link_libraries(server PRIVATE 
    chess_engine
    nlohmann_json::nlohmann_json
)

# Set include directories
target_include_directories(chess_engine PUBLIC src)
target_include_directories(server PUBLIC src)

# Add nlohmann/json include path
target_include_directories(chess_engine PUBLIC ${nlohmann_json_SOURCE_DIR}/include)
target_include_directories(server PUBLIC ${nlohmann_json_SOURCE_DIR}/include)

# Add Crow include path - Not needed for simple server
# target_include_directories(server PUBLIC ${crow_SOURCE_DIR}/include)