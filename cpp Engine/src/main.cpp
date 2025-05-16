#include <iostream>
#include <SFML/Graphics.hpp>

#include "board.hpp"
#include "move.hpp"

// cd build
// cmake ..
// make
// ./chess-engine

const int SQUARE_SIZE = 60;
const int BOARD_SIZE = 8;
const int TEXTURE_SQUARE_SIZE = 333;
const sf::Color BROWN = sf::Color(240, 217, 181);
const sf::Color TAN = sf::Color(181, 136, 99);
const int PIECES_MAP[] = { 5, 3, 2, 4, 1, 0 };

int main() {
    Board board;
    board.print();

    std::vector<Move> moves = board.get_moves();
    std::cout << "Valid moves:\n";
    for (const Move& move : moves) {
        std::cout << move.to_string() << "\n";
    }

    sf::RenderWindow window(sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE), "ChessLi");
    sf::Texture piece_texture;
    if (!piece_texture.loadFromFile("pieces.png")) return -1;

    sf::Sprite piece_sprite(piece_texture);
    piece_sprite.setScale(
        static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE,
        static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE
    );

    int selected_square = -1, clicked_square;
    int file, rank;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                file = event.mouseButton.x / SQUARE_SIZE;
                rank = 7 - (event.mouseButton.y / SQUARE_SIZE);
                clicked_square = rank * 8 + file;

                if (board.get_piece(clicked_square).is_empty()) continue;

                std::cout << "Selected square: " << file << ", " << rank << "\n";
                std::cout << "Piece: " << board.get_piece(clicked_square).to_char() << "\n";
                selected_square = clicked_square;
            }
        }
        window.clear();

        // Draw board squares
        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 8; ++file) {
                sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
                square.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
                square.setFillColor((file + rank) % 2 == 0 ? BROWN : TAN);
                window.draw(square);
            }
        }

        // Draw pieces
        Piece piece;
        int piece_location;
        Turn piece_color; 
        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                piece = board.get_piece(file, rank);
                if (piece.is_empty()) continue;
                piece_location = PIECES_MAP[piece.get_piece()];
                piece_color = piece.get_color();

                piece_sprite.setTextureRect(sf::IntRect(
                    piece_location * TEXTURE_SQUARE_SIZE,
                    piece_color * TEXTURE_SQUARE_SIZE, 
                    TEXTURE_SQUARE_SIZE, 
                    TEXTURE_SQUARE_SIZE
                ));
                piece_sprite.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
                window.draw(piece_sprite);
            }
        }

        // Draw selected square
        if (selected_square != -1) {
            file = selected_square % 8;
            rank = selected_square / 8;
            sf::RectangleShape selected_square_shape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
            selected_square_shape.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
            selected_square_shape.setFillColor(sf::Color(255, 0, 0, 100));
            window.draw(selected_square_shape);
        }

        // Draw valid moves
        if (selected_square != -1) {
            for (const Move& move : moves) {
                if (move.start() == selected_square) {
                    file = move.end() % 8;
                    rank = move.end() / 8;
                    sf::RectangleShape valid_move_shape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
                    valid_move_shape.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
                    valid_move_shape.setFillColor(sf::Color(0, 255, 0, 100));
                    window.draw(valid_move_shape);
                }
            }
        }

        window.display();
    }

    return 0;
}