#pragma once
#include <cstdint>

struct Bitboard {
    uint64_t bitboard;

    /** 
     * @brief Constructs a new Bitboard object.
     */
    constexpr Bitboard(uint64_t bits = 0) : bitboard(bits) {}

    void set(uint64_t bits) {
        bitboard = bits;
    }

    /**
     * @brief Finds the intersection of two bitboards.
     * 
     * @param a
     * @param b 
     * @return a Bitboard that is the intersection of a and b.
     */
    static constexpr Bitboard intersect_with(const Bitboard& a, const Bitboard& b) {
        return Bitboard(a.bitboard & b.bitboard);
    }

    /**
     * @brief Finds the union of two bitboards.
     * 
     * @param a
     * @param b 
     * @return a Bitboard that is the union of a and b.
     */
    static constexpr Bitboard union_with(const Bitboard& a, const Bitboard& b) {
        return Bitboard(a.bitboard | b.bitboard);
    }
};