from enum import IntEnum, Enum
from position import Position

class MoveType(Enum):
    MOVE = 0
    TAKE = 1

COLOR_DELIMITER = 6

class Piece(IntEnum):
    WHITE_PAWN = 0
    WHITE_KNIGHT = 1
    WHITE_BISHOP = 2
    WHITE_ROOK = 3
    WHITE_QUEEN = 4
    WHITE_KING = 5

    BLACK_PAWN = 6
    BLACK_KNIGHT = 7
    BLACK_BISHOP = 8
    BLACK_ROOK = 9
    BLACK_QUEEN = 10
    BLACK_KING = 11

class Move:
    def __init__(self, start: Position, end: Position, type: MoveType=None, piece: Piece=None):
        self.start = start
        self.end = end
        self.type = type
        self.piece = piece
    def __str__(self):
        if self.type == MoveType.MOVE:
            return f"From {self.start} move to {self.end}"
        elif self.type == MoveType.TAKE:
            return f"From {self.start} take on {self.end}"
        return self.__repr__()
    def __repr__(self):
        return f"From {self.start} to {self.end}"
    def __eq__(self, o):
        return self.start == o.start and self.end == o.end
    def __hash__(self):
        return hash((self.start, self.end))