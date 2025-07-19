#pragma once

#include <SFML/Graphics.hpp>
#include "board.hpp"
#include "move.hpp"

const int SQUARE_SIZE = 60;
const int CIRCLE_RADIUS = 25;
const int CIRCLE_START_OFFSET = (SQUARE_SIZE - (CIRCLE_RADIUS * 2)) / 2;
const int BOARD_SIZE = 8;
const int TEXTURE_SQUARE_SIZE = 333;
const sf::Color BROWN = sf::Color(240, 217, 181);
const sf::Color TAN = sf::Color(181, 136, 99);
const sf::Color RED = sf::Color(255, 0, 0, 50);
const sf::Color GREEN = sf::Color(0, 255, 0, 50);
const sf::Color BLUE = sf::Color(0, 0, 255, 50);

const int PIECES_MAP[] = { 5, 3, 2, 4, 1, 0 };

class ChessUI {
private:
    Board& board;
    std::vector<Move>& moves;
    
    // UI state variables
    int prev_sq = -1;
    bool waiting_for_promotion = false;
    Move pending_promotion_move;
    
    // SFML objects
    sf::RenderWindow& window;
    sf::Texture& piece_texture;
    sf::Sprite piece_sprite;

public:
    ChessUI(Board& board, std::vector<Move>& moves, sf::RenderWindow& window, sf::Texture& piece_texture)
        : board(board), moves(moves), window(window), piece_texture(piece_texture) {
        piece_sprite.setTexture(piece_texture);
        piece_sprite.setScale(
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE,
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE
        );
    }

    void handle_event(const sf::Event& event);
    void render();

private:
    void handle_promotion_input(const sf::Event& event);
    void handle_mouse_input(const sf::Event& event);
    void draw_square(const int file, const int rank, const sf::Color* color);
    void draw_circle(const int file, const int rank, const sf::Color* color);
    void draw_pieces();
    void highlight_selected_squares();
    bool is_selected(const int square) const;
    void get_file_rank(const int square, int* file, int* rank) const;
    bool is_a_move(const int start, const int end) const;
    Move get_move(const int start, const int end) const;
};