#include "board.hpp"
#include <iostream>
#include <iomanip>

Board::Board(const std::string fen) {
    reset();

    // set board pieces
    int i = 0, sq = 0;
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
            piece = Piece::from_fen(c);
            squares[sq] = piece;
            piece_bitboards[piece.get_color_int()][piece.get_piece_int()].add_square(sq);
            color_bitboards[piece.get_color_int()].add_square(sq);
            all_pieces_bitboard.add_square(sq);
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
    enemies = &color_bitboards[turn];
    friend_arr = &piece_bitboards[turn];
    enemy_arr = &piece_bitboards[turn];
}

void Board::print() const {
    for (int rank = 0; rank < 8; ++rank) {
        std::cout << "  " << std::string(33, '-') << "\n";
        std::cout << 8 - rank << " |";
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

std::vector<Move> Board::get_moves() const {
    std::vector<Move> moves;



    return moves;
}
