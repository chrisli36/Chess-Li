#pragma once
#include "bitboard.hpp"
#include "turn.hpp"

struct AttackBitboards {
    static constexpr int KNIGHT_DIRECTIONS[8][2] = {
        { 1,  2}, { 2,  1}, { 2, -1}, { 1, -2},
        {-1, -2}, {-2, -1}, {-2,  1}, {-1,  2}
    };
    static constexpr int KING_DIRECTIONS[8][2] = {
        { 1,  1}, { 1,  0}, { 1, -1}, { 0, -1},
        {-1, -1}, {-1,  0}, {-1,  1}, { 0,  1}
    };
    static constexpr bool is_valid_fr(const int file, const int rank, int* sq) {
        *sq = rank * 8 + file;
        return (file >= 0 && file < 8 && rank >= 0 && rank < 8);
    }

    static Bitboard compute_knight_attacks(const uint8_t sq) {
        const int rank = sq / 8;
        const int file = sq % 8;
        int new_rank, new_file, new_sq;
        Bitboard attacks = Bitboard();

        for (auto& dir : KNIGHT_DIRECTIONS) {
            new_rank = rank + dir[0];
            new_file = file + dir[1];
            if (!is_valid_fr(new_file, new_rank, &new_sq)) continue;
            attacks.add_square(new_sq);
        }
        return attacks;
    }
    static Bitboard compute_pawn_attacks(const Turn turn, const uint8_t sq) {
        const int rank = sq / 8;
        const int file = sq % 8;
        const int forward = (turn == Turn::WHITE) ? 1 : -1;
        int new_rank, new_file, new_sq;
        Bitboard attacks = Bitboard();

        // pawn captures
        for (int horizontal : {-1, 1}) {
            new_file = file + horizontal;
            new_rank = rank + forward;
            if (!is_valid_fr(new_file, new_rank, &new_sq)) continue;
            attacks.add_square(new_sq);
        }
        return attacks;
    }
    static Bitboard compute_king_attacks(const uint8_t sq) {
        const int rank = sq / 8;
        const int file = sq % 8;
        int new_rank, new_file, new_sq;
        Bitboard attacks = Bitboard();

        for (auto& dir : KING_DIRECTIONS) {
            new_rank = rank + dir[0];
            new_file = file + dir[1];
            if (!is_valid_fr(new_file, new_rank, &new_sq)) continue;
            attacks.add_square(new_sq);
        }
        return attacks;
    }
    static Bitboard compute_ray_between(const uint8_t sq1, const uint8_t sq2) {
        const int dx = sq2 % 8 - sq1 % 8;
        const int dy = sq2 / 8 - sq1 / 8;
        
        if (dx != 0 && dy != 0 && abs(dx) != abs(dy)) return Bitboard();

        const int step_x = (dx > 0) - (dx < 0);
        const int step_y = (dy > 0) - (dy < 0);

        Bitboard mask = Bitboard();
        int sq = sq1 + step_x + step_y * 8;
        while (sq != sq2) {
            mask.add_square(sq);
            sq += step_x + step_y * 8;
        }
        return mask;
    }

    static inline Bitboard knight_attacks[64];
    static inline Bitboard pawn_attacks[2][64];
    static inline Bitboard king_attacks[64];
    static inline Bitboard ray_between[64][64];

    static void init() {
        for (int sq = 0; sq < 64; ++sq) {
            knight_attacks[sq] = compute_knight_attacks(sq);
            pawn_attacks[Turn::WHITE][sq] = compute_pawn_attacks(Turn::WHITE, sq);
            pawn_attacks[Turn::BLACK][sq] = compute_pawn_attacks(Turn::BLACK, sq);
            king_attacks[sq] = compute_king_attacks(sq);
            for (int sq2 = 0; sq2 < 64; ++sq2) {
                ray_between[sq][sq2] = compute_ray_between(sq, sq2);
            }
        }
    }
};

inline const auto _attack_table_initializer = []() {
    AttackBitboards::init();
    return true;
}();