#pragma once

#include <SFML/Graphics.hpp>
#include "board.hpp"
#include "move.hpp"
#include "engine.hpp"

class ChessUI {
private:
    Board* board = nullptr;
    std::vector<Move> moves;
    
    // UI state variables
    int prev_sq = -1;
    bool waiting_for_promotion = false;
    Move pending_promotion_move;
    
    // Engine variables
    Engine* engine = nullptr;
    bool bot_mode = false;
    int engine_depth = 3;
    Turn player_color = Turn::WHITE;
    
    // SFML objects
    sf::RenderWindow& window;
    sf::Texture& piece_texture;
    sf::Sprite piece_sprite;

    // Constants
    int SQUARE_SIZE;
    int CIRCLE_RADIUS;
    int CIRCLE_START_OFFSET;
    static const int TEXTURE_SQUARE_SIZE = 333;
    static const sf::Color BROWN;
    static const sf::Color TAN;
    static const sf::Color RED;
    static const sf::Color GREEN;
    static const sf::Color BLUE;

    static constexpr int PIECES_MAP[6] = { 5, 3, 2, 4, 1, 0 };


public:
    ChessUI(Board* board, sf::RenderWindow& window, sf::Texture& piece_texture, int square_size, int circle_radius)
        : board(board), moves(board->get_moves()), window(window), piece_texture(piece_texture), 
          piece_sprite(piece_texture), SQUARE_SIZE(square_size), CIRCLE_RADIUS(circle_radius) {
        CIRCLE_START_OFFSET = (SQUARE_SIZE - (CIRCLE_RADIUS * 2)) / 2;
        piece_sprite.setScale(sf::Vector2f(
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE,
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE
        ));
    }

    ChessUI(Board* board, sf::RenderWindow& window, sf::Texture& piece_texture, int square_size, int circle_radius, 
            Engine* engine, int depth = 3, Turn player_color = Turn::WHITE)
        : board(board), moves(board->get_moves()), window(window), piece_texture(piece_texture), 
          piece_sprite(piece_texture), SQUARE_SIZE(square_size), CIRCLE_RADIUS(circle_radius), 
            engine(engine), engine_depth(depth), player_color(player_color) {
        CIRCLE_START_OFFSET = (SQUARE_SIZE - (CIRCLE_RADIUS * 2)) / 2;
        piece_sprite.setScale(sf::Vector2f(
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE,
            static_cast<float>(SQUARE_SIZE) / TEXTURE_SQUARE_SIZE
        ));
        bot_mode = true;
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
    void make_engine_move();
    void reset_state();
};