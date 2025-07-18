#include "chess_ui.hpp"

const sf::Color ChessUI::BROWN = sf::Color(240, 217, 181);
const sf::Color ChessUI::TAN = sf::Color(181, 136, 99);
const sf::Color ChessUI::RED = sf::Color(255, 0, 0, 50);
const sf::Color ChessUI::GREEN = sf::Color(0, 255, 0, 50);
const sf::Color ChessUI::BLUE = sf::Color(0, 0, 255, 50);

void ChessUI::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::Closed) {
        window.close();
        return;
    }
    
    if (waiting_for_promotion) {
        handle_promotion_input(event);
    } else {
        handle_mouse_input(event);
    }
}

void ChessUI::render() {
    window.clear();

    // Draw board squares
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            draw_square(file, rank, (file + rank) % 2 == 0 ? &BROWN : &TAN);

            // highlight controlled squares
            if (board.is_controlled(file + rank * 8)) {
                draw_circle(file, rank, &BLUE);
            }
        }
    }

    // Draw pieces
    draw_pieces();

    // highlight selected squares
    highlight_selected_squares();

    window.display();
}

void ChessUI::handle_promotion_input(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        MoveFlag promotion_flag = MoveFlag::NO_FLAG;
        
        switch (event.key.code) {
            case sf::Keyboard::Num0:
            case sf::Keyboard::Numpad0:
                promotion_flag = MoveFlag::QUEEN_PROMOTION;
                break;
            case sf::Keyboard::Num1:
            case sf::Keyboard::Numpad1:
                promotion_flag = MoveFlag::ROOK_PROMOTION;
                break;
            case sf::Keyboard::Num2:
            case sf::Keyboard::Numpad2:
                promotion_flag = MoveFlag::BISHOP_PROMOTION;
                break;
            case sf::Keyboard::Num3:
            case sf::Keyboard::Numpad3:
                promotion_flag = MoveFlag::KNIGHT_PROMOTION;
                break;
            default:
                break;
        }
        
        if (promotion_flag != MoveFlag::NO_FLAG) {
            // Create the promotion move with the selected piece
            Move promotion_move(pending_promotion_move.start(), pending_promotion_move.end(), promotion_flag);
            board.make_move(&promotion_move);
            moves = board.get_moves();
            prev_sq = -1;
            waiting_for_promotion = false;
        }
    }
}

void ChessUI::handle_mouse_input(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        int file = event.mouseButton.x / SQUARE_SIZE;
        int rank = 7 - (event.mouseButton.y / SQUARE_SIZE);
        int curr_sq = rank * 8 + file;

        if (is_selected(prev_sq)) {
            if (curr_sq == prev_sq) {
                prev_sq = -1;
            } else if (is_a_move(prev_sq, curr_sq)) {
                Move move = get_move(prev_sq, curr_sq);
                if (move.is_promotion()) {
                    // Store the pending move and wait for promotion input
                    pending_promotion_move = move;
                    waiting_for_promotion = true;
                    prev_sq = -1;
                } else {
                    // Normal move
                    board.make_move(&move);
                    moves = board.get_moves();
                    prev_sq = -1;
                }
            } else if (!board.is_empty(curr_sq)) {
                prev_sq = curr_sq;
            } else {
                prev_sq = -1;
            }
        } else if (!board.is_empty(curr_sq)) {
            prev_sq = curr_sq;
        }
    }
}

void ChessUI::draw_square(const int file, const int rank, const sf::Color* color) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    square.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
    square.setFillColor(*color);
    window.draw(square);
}

void ChessUI::draw_circle(const int file, const int rank, const sf::Color* color) {
    sf::CircleShape circle(CIRCLE_RADIUS);
    circle.setPosition(file * SQUARE_SIZE + CIRCLE_START_OFFSET, (7 - rank) * SQUARE_SIZE + CIRCLE_START_OFFSET);
    circle.setFillColor(*color);
    window.draw(circle);
}

void ChessUI::draw_pieces() {
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
}

void ChessUI::highlight_selected_squares() {
    if (is_selected(prev_sq)) {
        int file, rank;
        get_file_rank(prev_sq, &file, &rank);
        draw_square(file, rank, &RED);
        
        for (Move move : moves) {
            if (move.start() == prev_sq) {
                file = move.end() % 8;
                rank = move.end() / 8;
                draw_square(file, rank, &GREEN);
            }
        }
    }
}

bool ChessUI::is_selected(const int square) const {
    return square != -1;
}

void ChessUI::get_file_rank(const int square, int* file, int* rank) const {
    *file = square % 8;
    *rank = square / 8;
}

bool ChessUI::is_a_move(const int start, const int end) const {
    for (const Move& move : moves) {
        if (move.start() == start && move.end() == end) {
            return true;
        }
    }
    return false;
}

Move ChessUI::get_move(const int start, const int end) const {
    for (const Move& move : moves) {
        if (move.start() == start && move.end() == end) {
            return move;
        }
    }
    return Move();
}