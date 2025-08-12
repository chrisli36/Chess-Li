#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <chrono>

#include "board.hpp"
#include "chess_ui.hpp"
#include "engine.hpp"

const int BOARD_SIZE = 8;
const int SQUARE_SIZE = 60;
const int CIRCLE_RADIUS = 25;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [MODE] [OPTIONS]\n";
    std::cout << "Modes:\n";
    std::cout << "  board     - Regular chess board (default)\n";
    std::cout << "  bot       - Play against the chess engine\n";
    std::cout << "  test      - Test move generation speed\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --fen <FEN>  - Start with custom FEN position\n";
    std::cout << "  --depth <N>  - Search depth for test mode (default: 6)\n";
    std::cout << "  --engine-depth <N> - Engine search depth for bot mode (default: 3)\n";
    std::cout << "  --color <COLOR> - Player color in bot mode: 'white' or 'black' (default: white)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << "                   # Regular board\n";
    std::cout << "  " << program_name << " bot               # Play against bot\n";
    std::cout << "  " << program_name << " bot --engine-depth 4  # Play against bot with depth 4\n";
    std::cout << "  " << program_name << " bot --color black  # Play as black against bot\n";
    std::cout << "  " << program_name << " test --depth 6   # Test with depth 6\n";
    std::cout << "  " << program_name << " board --fen \"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\n";
}

void run_regular_board(const std::string& fen) {
    std::cout << "Starting regular chess board mode...\n";
    
    Board board(fen);
    
    sf::RenderWindow window(sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE), "ChessLi - Board Mode");
    
    sf::Texture piece_texture;
    if (!piece_texture.loadFromFile("pieces.png")) {
        std::cerr << "Error: Could not load pieces.png\n";
        return;
    }
    
    ChessUI ui(&board, window, piece_texture, SQUARE_SIZE, CIRCLE_RADIUS);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ui.handle_event(event);
        }
        ui.render();
    }
}

void run_bot_mode(const std::string& fen, int engine_depth, const Turn player_color) {
    std::cout << "Starting bot mode - you play against the chess engine...\n";
    std::cout << "Engine depth: " << engine_depth << "\n";
    std::cout << "You play as: " << (player_color == Turn::WHITE ? "white" : "black") << "\n";
    
    Board board(fen);
    Engine engine(&board);
    
    sf::RenderWindow window(sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE), "ChessLi - Bot Mode");
    
    sf::Texture piece_texture;
    if (!piece_texture.loadFromFile("pieces.png")) {
        std::cerr << "Error: Could not load pieces.png\n";
        return;
    }
    
    ChessUI ui(&board, window, piece_texture, SQUARE_SIZE, CIRCLE_RADIUS, &engine, engine_depth, player_color);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ui.handle_event(event);
        }
        ui.render();
    }
}

void run_test_mode(int max_depth) {
    std::cout << "Starting test mode - evaluating move generation speed...\n";
    std::cout << "FEN: " << Board::STARTING_BOARD << "\n";
    std::cout << "Testing depths 1 to " << max_depth << "...\n\n";
    
    Board board;
    Engine engine(&board);
    
    for (int depth = 1; depth <= max_depth; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        int result = engine.search(depth);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Depth " << depth << ": " << result << " (";
        if (duration.count() < 1000) {
            std::cout << duration.count() << "ms";
        } else {
            std::cout << (duration.count() / 1000.0) << "s";
        }
        std::cout << ")\n";
    }
}

int main(int argc, char* argv[]) {
    std::string mode = "board";
    std::string fen = Board::STARTING_BOARD;
    int depth = Engine::DEFAULT_DEPTH;
    Turn player_color = Turn::WHITE;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--fen") {
            if (i + 1 < argc) {
                fen = argv[++i];
            } else {
                std::cerr << "Error: --fen requires a FEN string\n";
                return 1;
            }
        } else if (arg == "--depth") {
            if (i + 1 < argc) {
                try {
                    depth = std::stoi(argv[++i]);
                    if (depth < 1 || depth > 12) {
                        std::cerr << "Error: Depth must be between 1 and 12\n";
                        return 1;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid depth value\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --depth requires a number\n";
                return 1;
            }
        } else if (arg == "--color") {
            if (i + 1 < argc) {
                std::string color = argv[++i];
                if (color == "white" || color == "w") {
                    player_color = Turn::WHITE;
                } else if (color == "black" || color == "b") {
                    player_color = Turn::BLACK;
                } else {
                    std::cerr << "Error: Color must be 'white' or 'black'\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --color requires 'white' or 'black'\n";
                return 1;
            }
        } else if (arg == "board" || arg == "bot" || arg == "test") {
            mode = arg;
        } else {
            std::cerr << "Error: Unknown argument '" << arg << "'\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    std::cout << "ChessLi - Chess Engine\n";
    std::cout << "Mode: " << mode << "\n";
    std::cout << "FEN: " << fen << "\n";
    if (mode == "test") {
        std::cout << "Test Depth: " << depth << "\n";
    }
    std::cout << "\n";
    
    try {
        if (mode == "board") {
            run_regular_board(fen);
        } else if (mode == "bot") {
            run_bot_mode(fen, depth, player_color);
        } else if (mode == "test") {
            run_test_mode(depth);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}