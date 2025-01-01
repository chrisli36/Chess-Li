RANKS = [8, 7, 6, 5, 4, 3, 2, 1]
FILES = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']

class Position:
    def __init__(self, num=0, pos=None):
        if num < 0 or num >= 64:
            raise RuntimeError
        self.num = num
        if pos is None:
            self.pos = 1 << num
        else:
            self.pos = pos
    def __str__(self):
        rank = self.num // 8
        file = self.num % 8
        return f"{FILES[file]}{RANKS[rank]}"
    def __repr__(self):
        return self.__str__()
    def __eq__(self, o):
        return self.num == o.num
    def __hash__(self):
        return self.num
    def next(self):
        if self.num >= 63:
            return None
        return Position(self.num + 1, self.pos << 1)
    def get_position(move: str):
        return Position(num=(8 - int(move[1])) * 8 + ord(move[0]) - ord('a'))
    def overlap(self, mask: int) -> int:
        return self.pos & mask
    def rank(self) -> int:
        return 8 - self.num // 8
    def file(self) -> int:
        return self.num % 8
    
    def right(self):
        if self.file() == 7:
            return None
        return Position(self.num + 1, self.pos << 1)
    def left(self):
        if self.file() == 0:
            return None
        return Position(self.num - 1, self.pos >> 1)
    def up(self):
        if self.rank() == 8:
            return None
        return Position(self.num - 8, self.pos >> 8)
    def down(self):
        if self.rank() == 1:
            return None
        return Position(self.num + 8, self.pos << 8)
    
    def up_right(self):
        if self.rank() == 8 or self.file() == 7:
            return None
        return Position(self.num - 7, self.pos >> 7)
    def up_left(self):
        if self.rank() == 8 or self.file() == 0:
            return None
        return Position(self.num - 9, self.pos >> 9)
    def down_left(self):
        if self.rank() == 1 or self.file() == 0:
            return None
        return Position(self.num + 7, self.pos << 7)
    def down_right(self):
        if self.rank() == 1 or self.file() == 7:
            return None
        return Position(self.num + 9, self.pos << 9)
    
    def up_up_right(self):
        if self.rank() >= 7 or self.file() == 7:
            return None
        return Position(self.num - 15, self.pos >> 15)
    def up_right_right(self):
        if self.rank() == 8 or self.file() >= 6:
            return None
        return Position(self.num - 6, self.pos >> 6)
    def down_right_right(self):
        if self.rank() == 1 or self.file() >= 6:
            return None
        return Position(self.num + 10, self.pos << 10)
    def down_down_right(self):
        if self.rank() <= 2 or self.file() == 7:
            return None
        return Position(self.num + 17, self.pos << 17)
    def down_down_left(self):
        if self.rank() <= 2 or self.file() == 0:
            return None
        return Position(self.num + 15, self.pos << 15)
    def down_left_left(self):
        if self.rank() == 1 or self.file() <= 1:
            return None
        return Position(self.num + 6, self.pos << 6)
    def up_left_left(self):
        if self.rank() == 8 or self.file() <= 1:
            return None
        return Position(self.num - 10, self.pos >> 10)
    def up_up_left(self):
        if self.rank() >= 7 or self.file() == 0:
            return None
        return Position(self.num - 17, self.pos >> 17)