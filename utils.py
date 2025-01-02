from enum import IntEnum

COLOR_DELIMITER = 6

class Piece(IntEnum):
    WHITE_PAWN = 0;     PAWN = 0
    WHITE_KNIGHT = 1;   KNIGHT = 1
    WHITE_BISHOP = 2;   BISHOP = 2
    WHITE_ROOK = 3;     ROOK = 3
    WHITE_QUEEN = 4;    QUEEN = 4
    WHITE_KING = 5;     KING = 5

    BLACK_PAWN = 6
    BLACK_KNIGHT = 7
    BLACK_BISHOP = 8
    BLACK_ROOK = 9
    BLACK_QUEEN = 10
    BLACK_KING = 11

    EMPTY = 12

PIECES = [
    Piece.WHITE_PAWN, 
    Piece.WHITE_KNIGHT, 
    Piece.WHITE_BISHOP, 
    Piece.WHITE_ROOK,
    Piece.WHITE_QUEEN, 
    Piece.WHITE_KING,
    Piece.BLACK_PAWN, 
    Piece.BLACK_KNIGHT, 
    Piece.BLACK_BISHOP, 
    Piece.BLACK_ROOK,
    Piece.BLACK_QUEEN, 
    Piece.BLACK_KING,
]

def ctzll(x):
    return (x & -x).bit_length() - 1 if x != 0 else 64

FILE = {
    'a': 0x0101010101010101,
    'b': 0x0202020202020202,
    'c': 0x0404040404040404,
    'd': 0x0808080808080808,
    'e': 0x1010101010101010,
    'f': 0x2020202020202020,
    'g': 0x4040404040404040,
    'h': 0x8080808080808080,
}

RANK = {
    1: 0x00000000000000FF,
    2: 0x000000000000FF00,
    3: 0x0000000000FF0000,
    4: 0x00000000FF000000,
    5: 0x000000FF00000000,
    6: 0x0000FF0000000000,
    7: 0x00FF000000000000,
    8: 0xFF00000000000000,
}