from position import Position
from utils import Piece, ctzll_iterator

class Moves:
    def __init__(self, mask:int=0, piece:Piece=None):
        self.mask = mask
        self.piece = piece
    def __str__(self):
        s = f"{self.piece}"
        for n in ctzll_iterator(self.mask):
            s += f" moves to {Position(n)};"
        return s
    def __repr__(self):
        return self.__str__()
    # def __eq__(self, o):
    #     return self.mask = o.mask
    def __hash__(self):
        return hash(self.mask)
    def update(self, position: Position):
        self.mask |= position.pos
    def limit_with_mask(self, mask: int):
        self.mask &= mask