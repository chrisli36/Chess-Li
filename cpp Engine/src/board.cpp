#include "board.hpp"
#include <iostream>
#include <iomanip>

Board::Board(const std::string fen) {
    reset();

    // set board pieces
    int i = 0, sq = 0, real_sq;
    Piece piece;
    for (char c : fen) {
        i++;
        if (sq == 64) {
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
            real_sq = (7 - sq / 8) * 8 + (sq % 8);
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
            case 'K': castling_rights.add_right(CastlingRights::K);
            case 'Q': castling_rights.add_right(CastlingRights::Q);
            case 'k': castling_rights.add_right(CastlingRights::k);
            case 'q': castling_rights.add_right(CastlingRights::q);
        }
        i++;
    }
    i++;

    // set en passant square
    if (fen[i] != '-') {
        int file = fen[i] - 'a';
        i++;
        int rank = 8 - (fen[i] - '0');
        en_passant_square = Bitboard(1ULL << (rank * 8 + file));
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
}

void Board::update_turn() {
    castle_king = (turn == Turn::WHITE) ? castling_rights.can_castle(CastlingRights::K) : castling_rights.can_castle(CastlingRights::k);
    castle_queen = (turn == Turn::WHITE) ? castling_rights.can_castle(CastlingRights::Q) : castling_rights.can_castle(CastlingRights::q);
    friends = &color_bitboards[turn];
    enemies = &color_bitboards[!turn];
    friend_arr = piece_bitboards[turn];
    enemy_arr = piece_bitboards[!turn];
}

bool Board::is_valid_fr(const int file, const int rank) const {
    return (file >= 0 && file < 8 && rank >= 0 && rank < 8);
}

void Board::print() const {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << "  " << std::string(33, '-') << "\n";
        std::cout << rank << " |";
        for (int file = 0; file < 8; ++file) {
            std::cout << " " << squares[rank * 8 + file].to_char() << " |";
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
    if (en_passant_square.bitboard) {
        int file = __builtin_ctzll(en_passant_square.bitboard) % 8;
        int rank = 8 - (__builtin_ctzll(en_passant_square.bitboard) / 8);
        std::cout << static_cast<char>('a' + file) << rank;
    } else {
        std::cout << "-";
    }
    std::cout << "\n";
}

std::vector<Move> Board::get_moves() {
    moves.clear();

    uint64_t x = friend_arr[Piece::KNIGHT].bitboard;
    uint8_t sq;

    CTZLL_ITERATOR(sq, x) {
        knight_moves(sq);
    }
    return moves;
}

void Board::knight_moves(const uint8_t sq) {
    const int rank = sq / 8;
    const int file = sq % 8;
    int new_rank, new_file, new_sq;
    for (auto& dir : KNIGHT_DIRECTIONS) {
        new_rank = rank + dir[0];
        new_file = file + dir[1];
        if (!is_valid_fr(new_file, new_rank)) continue;

        new_sq = new_rank * 8 + new_file;
        if (squares[new_sq].get_piece() == Piece::EMPTY || squares[new_sq].get_color() != turn) {
            moves.push_back(Move(sq, new_sq));
        }
    }
}
