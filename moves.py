from position import Position
from utils import Piece, ctzll

class Moves:
    def __init__(self, mask:int=0, piece:Piece=None):
        self.mask = mask
        self.piece = piece
    def __str__(self):
        s = f"{self.piece}"
        m = self.mask
        while m > 0:
            s += f" move to {Position(num=ctzll(m))},"
            m &= m - 1
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