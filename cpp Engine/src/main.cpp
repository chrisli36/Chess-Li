#include <iostream>
#include <SFML/Graphics.hpp>

#include "board.hpp"
#include "move.hpp"
#include "chess_ui.hpp"

// cd build
// cmake ..
// make
// ./chess-engine

int main() {
    Board board;
    std::vector<Move> moves = board.get_moves();

    // board.print();
    // std::cout << "Valid moves:\n";
    // for (const Move& move : moves) {
    //     std::cout << move.to_string() << "\n";
    // }

    const int BOARD_SIZE = 8;
    const int SQUARE_SIZE = 60;
    const int CIRCLE_RADIUS = 25;
    sf::RenderWindow window(sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE), "ChessLi");

    sf::Texture piece_texture;
    if (!piece_texture.loadFromFile("pieces.png")) return -1;

    ChessUI ui(board, moves, window, piece_texture, SQUARE_SIZE, CIRCLE_RADIUS);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ui.handle_event(event);
        }
        ui.render();
    }

    return 0;
}