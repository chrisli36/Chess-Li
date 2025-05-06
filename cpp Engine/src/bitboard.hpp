#pragma once
#include <cstdint>

struct Bitboard {
    uint64_t bitboard;

    /** 
     * @brief Constructs a new Bitboard object.
     */
    constexpr Bitboard(uint64_t bits = 0) : bitboard(bits) {}

    /**
     * @brief Resets the bitboard to 0.
     * 
     */
    constexpr void reset() {
        bitboard = 0ULL;
    }

    /**
     * @brief Sets the bitboard to a specific value.
     * 
     * @param bits The value to set the bitboard to.
     */
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
     * @brief Intersects the bitboard with another bitboard.
     * 
     * @param a
     */
    constexpr void intersect_with(const Bitboard& a) {
        bitboard &= a.bitboard;
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

    /**
     * @brief Unions the bitboard with another bitboard.
     * 
     * @param a
     */
    constexpr void union_with(const Bitboard& a) {
        bitboard |= a.bitboard;
    }

    /**
     * @brief Adds a piece to the bitboard
     * 
     * @param sq The square to add the piece to.
     */
    constexpr void add_square(uint8_t sq) {
        bitboard |= (1ULL << sq);
    }
};