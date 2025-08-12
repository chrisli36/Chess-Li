#include "board.hpp"
#include "move.hpp"
#include "position.hpp"
#include "attacks.hpp"
#include <iostream>
#include <cassert>

Board::Board(const std::string fen) {
    reset();

    // set board pieces
    int i = 0, sq = 0, real_sq;
    Piece piece;
    for (char c : fen) {
        i++;
        if (sq == BOARD_SQUARES) {
            break;
        }
        if (c == '/') {
            continue;
        }
        if (std::isdigit(c)) {
            for (int j = sq; j < sq + (c - '0'); ++j) {
                squares[j] = Piece::EMPTY;
            }
            sq += (c - '0');
        } else {
            real_sq = (BOARD_SIZE - 1 - sq / BOARD_SIZE) * BOARD_SIZE + (sq % BOARD_SIZE);
            piece = Piece::from_fen(c);
            squares[real_sq] = piece;
            piece_bitboards[piece.get_color()][piece.get_piece()].add_square(real_sq);
            color_bitboards[piece.get_color()].add_square(real_sq);
            all_pieces_bitboard.add_square(real_sq);
            sq++;
        }
    }

    // set turn
    turn = (fen[i] == 'w') ? Turn::WHITE : Turn::BLACK;
    i++; i++;

    // set castling rights
    castling_rights = CastlingRights(0);
    while (fen[i] != ' ') {
        switch (fen[i]) {
            case 'K': castling_rights.add_right(CastlingRights::K); break;
            case 'Q': castling_rights.add_right(CastlingRights::Q); break;
            case 'k': castling_rights.add_right(CastlingRights::k); break;
            case 'q': castling_rights.add_right(CastlingRights::q); break;
        }
        i++;
    }
    i++;

    // set en passant square
    if (fen[i] != '-') {
        int file = fen[i] - 'a';
        i++;
        int rank = BOARD_SIZE - (fen[i] - '0');
        en_passant_square = Bitboard(1ULL << (rank * BOARD_SIZE + file));
    } else {
        en_passant_square = Bitboard(0);
    }

    update_turn();
}

void Board::reset() {
    for (int i = 0; i < 64; ++i) {
        squares[i].reset();
    }
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 6; ++j) {
            piece_bitboards[i][j].reset();
        }
        color_bitboards[i].reset();
    }
    all_pieces_bitboard.reset();
    castling_rights = CastlingRights();
    turn = Turn::WHITE;
    en_passant_square.reset();
    controlled_squares.reset();
    attacker_count = 0;
    attackers[0] = Position::INVALID_SQUARE;
    attackers[1] = Position::INVALID_SQUARE;
    calculated = false;
}

void Board::update_turn() {
    castle_king = (turn == Turn::WHITE) ? castling_rights.can_castle(CastlingRights::K) : castling_rights.can_castle(CastlingRights::k);
    castle_queen = (turn == Turn::WHITE) ? castling_rights.can_castle(CastlingRights::Q) : castling_rights.can_castle(CastlingRights::q);
    friends = &color_bitboards[turn];
    enemies = &color_bitboards[!turn];
    friend_arr = piece_bitboards[turn];
    enemy_arr = piece_bitboards[!turn];
}

bool Board::is_valid_fr(const int file, const int rank, int* sq) const {
    *sq = rank * BOARD_SIZE + file;
    return (static_cast<unsigned>(file) < BOARD_SIZE && static_cast<unsigned>(rank) < BOARD_SIZE);
}

void Board::print() const {
    for (int rank = BOARD_SIZE - 1; rank >= 0; --rank) {
        std::cout << "  " << std::string(33, '-') << "\n";
        std::cout << rank << " |";
        for (int file = 0; file < BOARD_SIZE; ++file) {
            std::cout << " " << squares[rank * BOARD_SIZE + file].to_char() << " |";
        }
        std::cout << "\n";
    }
    std::cout << "  " << std::string(33, '-') << "\n";
    std::cout << "    a   b   c   d   e   f   g   h\n";
    std::cout << "Turn: " << (turn == Turn::WHITE ? "White" : "Black") << "\n";
    std::cout << "Castling Rights: ";
    if (castling_rights.can_castle(CastlingRights::K)) std::cout << "K ";
    if (castling_rights.can_castle(CastlingRights::Q)) std::cout << "Q ";
    if (castling_rights.can_castle(CastlingRights::k)) std::cout << "k ";
    if (castling_rights.can_castle(CastlingRights::q)) std::cout << "q ";
    std::cout << "\n";
    std::cout << "En Passant: ";
    if (en_passant_square) {
        int file = __builtin_ctzll(en_passant_square) % BOARD_SIZE;
        int rank = BOARD_SIZE - (__builtin_ctzll(en_passant_square) / BOARD_SIZE);
        std::cout << static_cast<char>('a' + file) << rank;
    } else {
        std::cout << "-";
    }
    std::cout << "\n";
}

bool Board::two_pawns_en_passant(const int file, const int rank, const int dir[2], int* two_pawns_sq) const {
    int next_rank = rank + dir[0];
    int next_file = file + dir[1];
    *two_pawns_sq = Position::INVALID_SQUARE;

    int sq2;
    if (!is_valid_fr(next_file, next_rank, &sq2)) return false;
    const int sq1 = rank * BOARD_SIZE + file;

    const bool same_rank = rank == next_rank;
    if (!same_rank) return false;

    const bool both_are_pawns = squares[sq1].get_piece() == Piece::PAWN && squares[sq2].get_piece() == Piece::PAWN;
    if (!both_are_pawns) return false;

    const bool opposite_colors = squares[sq1].get_color() != squares[sq2].get_color();
    if (!opposite_colors) return false;

    const uint8_t friend_sq = (turn == squares[sq1].get_color()) ? sq1 : sq2;
    const uint8_t enemy_sq = (turn == squares[sq1].get_color()) ? sq2 : sq1;
    const uint8_t enemy_sq_back_one = enemy_sq + ((turn == Turn::WHITE) ? PAWN_MOVE_ONE : -PAWN_MOVE_ONE);
    const bool enemy_just_moved_two = en_passant_square.covers(enemy_sq_back_one);
    if (!enemy_just_moved_two) return false;

    *two_pawns_sq = friend_sq;
    return true;
}

bool Board::is_aligned(const int dir[2], const Piece piece) const {
    return (piece.get_piece() == Piece::QUEEN)
        || (piece.get_piece() == Piece::ROOK && dir[0] * dir[1] == 0)
        || (piece.get_piece() == Piece::BISHOP && dir[0] * dir[1] != 0);
}

void Board::calculate_pins() {
    const int king_sq = __builtin_ctzll(friend_arr[Piece::KING]);
    const int king_rank = king_sq / BOARD_SIZE;
    const int king_file = king_sq % BOARD_SIZE;
    int new_rank, new_file, new_sq;
    int two_pawns_sq;
    Piece new_piece;
    Bitboard pinned_ray = Bitboard();
    for (int i = 0; i < BOARD_SQUARES; ++i) {
        pinned_limits[i].reset();
    }
    int pinned_piece_sq;
    
    for (auto& dir : KING_DIRECTIONS) {
        pinned_ray.reset();
        new_rank = king_rank + dir[0];
        new_file = king_file + dir[1];
        pinned_piece_sq = Position::INVALID_SQUARE;
        two_pawns_sq = Position::INVALID_SQUARE;
        while (is_valid_fr(new_file, new_rank, &new_sq)) {
            pinned_ray.add_square(new_sq);
            new_piece = squares[new_sq];
            if (!new_piece.is_empty()) {
                if (two_pawns_sq == Position::INVALID_SQUARE
                        && two_pawns_en_passant(new_file, new_rank, dir, &two_pawns_sq)) {
                    // std::cout << "Two pawns en passant, pinned pawn found at " << Move::to_algebraic(two_pawns_sq) << std::endl;
                    new_rank += dir[0];
                    new_file += dir[1];
                } else if (new_piece.is_friendly(turn)) {
                    if (pinned_piece_sq == Position::INVALID_SQUARE && two_pawns_sq == Position::INVALID_SQUARE) {
                        pinned_piece_sq = new_sq;
                    } else {
                        break;
                    }
                } else if (is_aligned(dir, new_piece)) {
                    if (pinned_piece_sq != Position::INVALID_SQUARE) {
                        pinned_limits[pinned_piece_sq] = pinned_ray;
                    } else if (two_pawns_sq != Position::INVALID_SQUARE) {
                        // std::cout << "setting pinned limit for " << Move::to_algebraic(two_pawns_sq) << " to " << ~en_passant_square << std::endl;
                        pinned_limits[two_pawns_sq] = ~en_passant_square;
                    }
                    break;
                } else {
                    break;
                }
            }
            new_rank += dir[0];
            new_file += dir[1];
        }
    }
}

GameState Board::get_game_state() {
    if (!calculated) calculate_moves();

    if (moves.empty()) {
        if (attacker_count == 0) {
            return GameState::DRAW;
        } else {
            return turn == Turn::WHITE ? GameState::BLACK_WIN : GameState::WHITE_WIN;
        }
    }
    return GameState::IN_PROGRESS;
}

std::vector<Move> Board::get_moves() {
    if (!calculated) calculate_moves();
    return moves;
}

void Board::calculate_moves() {
    if (calculated) return;
    uint8_t sq;
    moves.clear();
    evasion_mask = Bitboard(~0ULL);

    // calculate controlled squares
    controlled_squares.reset();
    attacker_count = 0;
    attackers[0] = Position::INVALID_SQUARE;
    attackers[1] = Position::INVALID_SQUARE;
    for (Piece::PieceType piece : PIECES) {
        CTZLL_ITERATOR(sq, enemy_arr[piece]) {
            (this->*CALCULATE_CONTROLLED_FUNCTIONS[piece])(sq);
        }
    }

    // calculate pins
    calculate_pins();

    // if the king is in double check, only return king moves
    if (attacker_count == 2) {
        const uint8_t king_sq = __builtin_ctzll(friend_arr[Piece::KING]);
        king_moves(king_sq);
        return;
    } else if (attacker_count == 1) {
        const uint8_t king_sq = __builtin_ctzll(friend_arr[Piece::KING]);
        const uint8_t attacker_sq = attackers[0];
        evasion_mask = AttackBitboards::ray_between[attacker_sq][king_sq];
        evasion_mask.add_square(attacker_sq);
    }

    // calculate moves, limited by checks and pins
    for (Piece::PieceType piece : PIECES) {
        CTZLL_ITERATOR(sq, friend_arr[piece]) {
            (this->*CALCULATE_MOVES_FUNCTIONS[piece])(sq);
        }
    }
}

void Board::erase_piece(const int sq) {
    if (squares[sq].is_empty()) {
        return;
    }
    Piece piece = squares[sq];
    squares[sq] = Piece::EMPTY;
    piece_bitboards[piece.get_color()][piece.get_piece()].remove_square(sq);
    color_bitboards[piece.get_color()].remove_square(sq);
    all_pieces_bitboard.remove_square(sq);
}

void Board::add_piece(const int sq, const Piece piece) {
    if (!squares[sq].is_empty()) {
        return;
    }
    squares[sq] = piece;
    piece_bitboards[piece.get_color()][piece.get_piece()].add_square(sq);
    color_bitboards[piece.get_color()].add_square(sq);
    all_pieces_bitboard.add_square(sq);
}

void Board::rook_disabling_castling_move(const uint8_t sq) {
    uint8_t right = 0;
    switch (sq) {
        case Position::A1: right = CastlingRights::Q; break;
        case Position::H1: right = CastlingRights::K; break;
        case Position::A8: right = CastlingRights::q; break;
        case Position::H8: right = CastlingRights::k; break;
        default: return;
    }

    castling_rights.remove_right(right);
}

void Board::make_move(const Move* move) {
    const uint8_t start = move->start();
    const uint8_t end = move->end();
    const Piece start_piece = squares[start];
    const Piece end_piece = squares[end];
    Bitboard prev_en_passant_square = en_passant_square;
    const CastlingRights prev_castling_rights = castling_rights;

    // move the piece to the new square, erasing the old square
    erase_piece(end);
    erase_piece(start);
    add_piece(end, start_piece);

    // account for special moves
    en_passant_square.reset();
    if (move->is_en_passant()) {
        // std::cout << "En passant capture." << std::endl;
        erase_piece(end + ((turn == Turn::WHITE) ? -PAWN_MOVE_ONE : PAWN_MOVE_ONE));
    } else if (move->is_pawn_up_two()) {
        // std::cout << "Pawn moved up two squares." << std::endl;
        en_passant_square = Bitboard(1ULL << (start + ((turn == Turn::WHITE) ? PAWN_MOVE_ONE : -PAWN_MOVE_ONE)));
    } else if (move->is_castle()) {
        // std::cout << "Castling move." << std::endl;
        if (move->is_castle_kingside()) {
            // std::cout << "Kingside castle." << std::endl;
            Piece rook = squares[end + 1];
            erase_piece(end + 1);
            add_piece(end - 1, rook);
        } else if (move->is_castle_queenside()) {
            // std::cout << "Queenside castle." << std::endl;
            Piece rook = squares[end - 2];
            erase_piece(end - 2);
            add_piece(end + 1, rook);
        }
    } else if (move->is_promotion()) {
        Piece promotion_piece = move->promotion_piece(turn);
        // std::cout << "Promotion move" << std::endl;
        erase_piece(end);
        add_piece(end, promotion_piece);
    }

    // check for anything that disables castling
    if (start_piece.get_piece() == Piece::KING) {
        if (turn == Turn::WHITE) {
            castling_rights.remove_right(CastlingRights::K);
            castling_rights.remove_right(CastlingRights::Q);
        } else {
            castling_rights.remove_right(CastlingRights::k);
            castling_rights.remove_right(CastlingRights::q);
        }
    } 
    if (start_piece.get_piece() == Piece::ROOK) rook_disabling_castling_move(start); 
    if (end_piece.get_piece() == Piece::ROOK) rook_disabling_castling_move(end);

    history.push_back(
        UnMove(
            Move(start, end, move->flag()), 
            end_piece,
            prev_en_passant_square,
            prev_castling_rights
        )
    );
    turn = static_cast<Turn>(!turn);
    update_turn();
    calculated = false;
}

void Board::undo_move() {
    if (history.empty()) return;

    const UnMove un_move = history.back();
    const Move move = un_move.move;
    const uint8_t start = move.start();
    const uint8_t end = move.end();

    const Piece taken_piece = un_move.taken_piece;
    const Piece start_piece = squares[end];

    // move the piece back to the old square
    erase_piece(end);
    add_piece(start, start_piece);
    if (!taken_piece.is_empty() && !move.is_en_passant()) {
        add_piece(end, taken_piece);
    }
    en_passant_square = un_move.en_passant_square;
    castling_rights = un_move.castling_rights;

    // account for special moves
    if (move.is_en_passant()) {
        // std::cout << "Undoing en passant capture." << std::endl;
        const Piece pawn = (turn == Turn::WHITE) ? Piece::WHITE_PAWN : Piece::BLACK_PAWN;
        const int actual_end = end + ((turn == Turn::WHITE) ? PAWN_MOVE_ONE : -PAWN_MOVE_ONE);
        add_piece(actual_end, pawn);
    } else if (move.is_pawn_up_two()) {
        // std::cout << "Undoing pawn moved up two squares." << std::endl;
    } else if (move.is_castle()) {
        // std::cout << "Undoing castling move." << std::endl;
        if (move.is_castle_kingside()) {
            // std::cout << "Undoing kingside castle." << std::endl;
            const Piece rook = squares[end - 1];
            erase_piece(end - 1);
            add_piece(end + 1, rook);
        } else if (move.is_castle_queenside()) {
            // std::cout << "Undoing queenside castle." << std::endl;
            const Piece rook = squares[end + 1];
            erase_piece(end + 1);
            add_piece(end - 2, rook);
        }
    } else if (move.is_promotion()) {
        // std::cout << "Undoing promotion move." << std::endl;
        const Piece pawn = (turn == Turn::WHITE) ? Piece::BLACK_PAWN : Piece::WHITE_PAWN;
        erase_piece(start);
        add_piece(start, pawn);
    }

    history.pop_back();
    turn = static_cast<Turn>(!turn);
    update_turn();
    calculated = false;
}

inline void Board::add_king_attacker(const uint8_t start, Bitboard attacks) {
    if (friend_arr[Piece::KING] & attacks) {
        assert(attacker_count < 2 && "Too many checkers!");
        // std::cout << "Adding king attacker: " << squares[start].to_char() << " on "<< Move::to_algebraic(start) << std::endl;
        attackers[attacker_count++] = start;
    }
}

inline bool Board::can_move_under_pin(const uint8_t sq, const uint8_t new_sq) {
    if (!pinned_limits[sq]) return true;
    // std::cout << "Checking if " << Move::to_algebraic(sq) << " can move to " << Move::to_algebraic(new_sq) << std::endl;
    // pinned_limits[sq].print();
    return pinned_limits[sq].covers(new_sq);
}

void Board::pawn_controlled(const uint8_t sq) {
    Bitboard attacks = AttackBitboards::pawn_attacks[!turn][sq];
    add_king_attacker(sq, attacks);
    controlled_squares |= attacks;
}

void Board::pawn_moves(const uint8_t sq) {
    const int rank = sq / BOARD_SIZE;
    const int file = sq % BOARD_SIZE;
    const int forward = (turn == Turn::WHITE) ? PAWN_FORWARD_WHITE : PAWN_FORWARD_BLACK;
    const bool is_white = (turn == Turn::WHITE);
    int new_rank, new_file, new_sq;

    // forward pawn moves
    new_rank = rank + forward;
    if (is_valid_fr(file, new_rank, &new_sq)
            && squares[new_sq].is_empty()
            && can_move_under_pin(sq, new_sq)) {
        // pawn up one
        if (evasion_mask.covers(new_sq)) {
            if ((rank == PAWN_PROMOTION_RANK_WHITE && is_white) || (rank == PAWN_PROMOTION_RANK_BLACK && !is_white)) {
                moves.push_back(Move(sq, new_sq, MoveFlag::QUEEN_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::ROOK_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::BISHOP_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::KNIGHT_PROMOTION));
            } else {
                moves.push_back(Move(sq, new_sq));
            }
        }
        // pawn up two
        if ((is_white && rank == PAWN_START_RANK_WHITE) || (!is_white && rank == PAWN_START_RANK_BLACK)) {
            new_rank = rank + forward + forward;
            if (is_valid_fr(file, new_rank, &new_sq) 
                    && squares[new_sq].is_empty()
                    && can_move_under_pin(sq, new_sq)) {
                if (evasion_mask.covers(new_sq)) {
                    moves.push_back(Move(sq, new_sq, MoveFlag::PAWN_UP_TWO));
                }
            }
        }
    }

    // pawn captures
    Bitboard attacks = AttackBitboards::pawn_attacks[turn][sq];
    CTZLL_ITERATOR(new_sq, attacks) {
        if (squares[new_sq].is_enemy(turn)
                && evasion_mask.covers(new_sq)
                && can_move_under_pin(sq, new_sq)) {
            if ((rank == PAWN_PROMOTION_RANK_WHITE && is_white) || (rank == PAWN_PROMOTION_RANK_BLACK && !is_white)) {
                moves.push_back(Move(sq, new_sq, MoveFlag::QUEEN_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::ROOK_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::BISHOP_PROMOTION));
                moves.push_back(Move(sq, new_sq, MoveFlag::KNIGHT_PROMOTION));
            } else {
                moves.push_back(Move(sq, new_sq));
            }
        } else if (squares[new_sq].is_empty()
                && en_passant_square.covers(new_sq)
                && evasion_mask.covers(new_sq)
                && can_move_under_pin(sq, new_sq)) {
            moves.push_back(Move(sq, new_sq, MoveFlag::EN_PASSANT_CAPTURE));
        }
    }
}

void Board::knight_controlled(const uint8_t sq) {
    Bitboard attacks = AttackBitboards::knight_attacks[sq];
    add_king_attacker(sq, attacks);
    controlled_squares |= attacks;
}

void Board::knight_moves(const uint8_t sq) {
    uint8_t new_sq;
    Bitboard attacks = AttackBitboards::knight_attacks[sq];

    // if knight is pinned, it definitely can't move
    if (pinned_limits[sq]) return;
    CTZLL_ITERATOR(new_sq, attacks) {
        if (!evasion_mask.covers(new_sq)) continue;
        if (squares[new_sq].is_empty() || squares[new_sq].is_enemy(turn)) {
            moves.push_back(Move(sq, new_sq));
        }
    }
}

void Board::bishop_controlled(const uint8_t sq) {
    const int rank = sq / BOARD_SIZE;
    const int file = sq % BOARD_SIZE;
    int new_rank, new_file, new_sq;
    Bitboard attacks = Bitboard();

    for (auto& dir : BISHOP_DIRECTIONS) {
        new_rank = rank + dir[0];
        new_file = file + dir[1];
        while (is_valid_fr(new_file, new_rank, &new_sq)) {
            attacks.add_square(new_sq);
            if (!squares[new_sq].is_empty() && squares[new_sq].get_piece() != Piece::KING) break;
            new_rank += dir[0];
            new_file += dir[1];
        }
    }
    add_king_attacker(sq, attacks);
    controlled_squares |= attacks;
}

void Board::bishop_moves(const uint8_t sq) {
    const int rank = sq / BOARD_SIZE;
    const int file = sq % BOARD_SIZE;
    int new_rank, new_file, new_sq;

    for (auto& dir : BISHOP_DIRECTIONS) {
        MOVE_ITERATOR(dir, rank, file, new_rank, new_file, new_sq) {
            if (!can_move_under_pin(sq, new_sq)) break;
            if (squares[new_sq].is_friendly(turn)) break;

            bool evasion_mask_covers = evasion_mask.covers(new_sq);

            if (squares[new_sq].is_empty()) {
                if (evasion_mask_covers) moves.push_back(Move(sq, new_sq));
                continue;
            } else if (squares[new_sq].is_enemy(turn)) {
                if (evasion_mask_covers) moves.push_back(Move(sq, new_sq));
                break;
            }
            throw std::runtime_error("Bishop move error, square " + std::to_string(new_sq) + " is not friendly, empty, or enemy");
        }
    }
}

void Board::rook_controlled(const uint8_t sq) {
    const int rank = sq / BOARD_SIZE;
    const int file = sq % BOARD_SIZE;
    int new_rank, new_file, new_sq;
    Bitboard attacks = Bitboard();

    for (auto& dir : ROOK_DIRECTIONS) {
        new_rank = rank + dir[0];
        new_file = file + dir[1];
        while (is_valid_fr(new_file, new_rank, &new_sq)) {
            attacks.add_square(new_sq);
            if (!squares[new_sq].is_empty() && squares[new_sq].get_piece() != Piece::KING) break;
            new_rank += dir[0];
            new_file += dir[1];
        }
    }
    add_king_attacker(sq, attacks);
    controlled_squares |= attacks;
}

void Board::rook_moves(const uint8_t sq) {
    const int rank = sq / BOARD_SIZE;
    const int file = sq % BOARD_SIZE;
    int new_rank, new_file, new_sq;

    for (auto& dir : ROOK_DIRECTIONS) {
        MOVE_ITERATOR(dir, rank, file, new_rank, new_file, new_sq) {
            if (!can_move_under_pin(sq, new_sq)) break;
            if (squares[new_sq].is_friendly(turn)) break;

            bool evasion_mask_covers = evasion_mask.covers(new_sq);

            if (squares[new_sq].is_empty()) {
                if (evasion_mask_covers) moves.push_back(Move(sq, new_sq));
                continue;
            } else if (squares[new_sq].is_enemy(turn)) {
                if (evasion_mask_covers) moves.push_back(Move(sq, new_sq));
                break;
            }
            throw std::runtime_error("Rook move error, square " + std::to_string(new_sq) + " is not friendly, empty, or enemy");
        }
    }
}

void Board::queen_controlled(const uint8_t sq) {
    bishop_controlled(sq);
    rook_controlled(sq);
}

void Board::queen_moves(const uint8_t sq) {
    bishop_moves(sq);
    rook_moves(sq);
}

void Board::king_controlled(const uint8_t sq) {
    Bitboard attacks = AttackBitboards::king_attacks[sq];
    add_king_attacker(sq, attacks);
    controlled_squares |= attacks;
}

void Board::king_moves(const uint8_t sq) {
    uint8_t new_sq;

    // calculate moves, limited by enemy controlled squares
    CTZLL_ITERATOR(new_sq, AttackBitboards::king_attacks[sq]) {
        if (controlled_squares.covers(new_sq)) continue;

        if (squares[new_sq].is_empty()) {
            moves.push_back(Move(sq, new_sq));
        } else if (squares[new_sq].is_enemy(turn)) {
            moves.push_back(Move(sq, new_sq));
        }
    }

    // check for castling
    if (attacker_count == 0) {
        const Bitboard blockers = all_pieces_bitboard | controlled_squares;
        if (castle_king && (KINGSIDE_CASTLE[turn] & blockers) == 0) {
            moves.push_back(Move(sq, sq + 2, MoveFlag::KINGSIDE_CASTLE));
        }
        if (castle_queen && (QUEENSIDE_CASTLE[turn] & blockers) == 0) {
            moves.push_back(Move(sq, sq - 2, MoveFlag::QUEENSIDE_CASTLE));
        }
    }
}