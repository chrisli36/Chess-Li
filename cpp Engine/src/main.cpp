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
const sf::Color RED = sf::Color(255, 0, 0, 100);
const sf::Color GREEN = sf::Color(0, 255, 0, 100);

const int PIECES_MAP[] = { 5, 3, 2, 4, 1, 0 };

void draw_square(sf::RenderWindow* window, const int file, const int rank, const int size, const sf::Color* color) {
    sf::RectangleShape square(sf::Vector2f(size, size));
    square.setPosition(file * size, (7 - rank) * size);
    square.setFillColor(*color);
    window->draw(square);
}

bool is_selected(const int square, int* file, int* rank) {
    *file = square % 8;
    *rank = square / 8;
    return square != -1;
}

bool is_a_move(std::vector<Move>* moves, const int start, const int end, Move* mv) {
    for (Move move : *moves) {
        if (move.start() == start && move.end() == end) {
            *mv = move;
            return true;
        }
    }
    return false;
}

int main() {
    Board board;
    std::vector<Move> moves = board.get_moves();

    // board.print();
    // std::cout << "Valid moves:\n";
    // for (const Move& move : moves) {
    //     std::cout << move.to_string() << "\n";
    // }

    sf::RenderWindow window(sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE), "ChessLi");
    sf::Texture piece_texture;
    if (!piece_texture.loadFromFile("pieces.png")) return -1;

    sf::Sprite piece_sprite(piece_texture);
    piece_sprite.setScale(
        static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE,
        static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE
    );

    int prev_sq = -1, curr_sq;
    int file, rank;
    Move move;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                file = event.mouseButton.x / SQUARE_SIZE;
                rank = 7 - (event.mouseButton.y / SQUARE_SIZE);
                curr_sq = rank * 8 + file;

                if (is_selected(prev_sq, &file, &rank)) {
                    // moves = board.get_moves();
                    if (curr_sq == prev_sq) {
                        prev_sq = -1;
                    } else if (is_a_move(&moves, prev_sq, curr_sq, &move)) {
                        board.make_move(&move);
                        moves = board.get_moves();
                        prev_sq = -1;
                    } else if (!board.is_empty(curr_sq)) {
                        prev_sq = curr_sq;
                    } else {
                        prev_sq = -1;
                    }
                } else if (!board.is_empty(curr_sq)) {
                    prev_sq = curr_sq;
                }

                // if (board.get_piece(curr_sq).is_empty()) continue;

                // std::cout << "Selected square: " << file << ", " << rank << "\n";
                // std::cout << "Piece: " << board.get_piece(clicked_square).to_char() << "\n";
                // selected_square = clicked_square;
            }
        }
        window.clear();

        // Draw board squares
        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 8; ++file) {
                draw_square(&window, file, rank, SQUARE_SIZE, (file + rank) % 2 == 0 ? &BROWN : &TAN);
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

        // Highlight selected square
        if (is_selected(prev_sq, &file, &rank)) {
            draw_square(&window, file, rank, SQUARE_SIZE, &RED);
            for (Move move : moves) {
                if (move.start() == prev_sq) {
                    file = move.end() % 8;
                    rank = move.end() / 8;
                    draw_square(&window, file, rank, SQUARE_SIZE, &GREEN);
                }
            }
        }

        window.display();
    }

    return 0;
}