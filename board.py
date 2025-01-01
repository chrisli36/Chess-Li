from position import *
from move import *
from enum import Flag
from utils import *
from magic import ROOK_MAGIC, QUEEN_MAGIC, BISHOP_MAGIC, KNIGHT_MAGIC

ROOK_DIRECTIONS = [Position.up, Position.down, Position.right, Position.left]
BISHOP_DIRECTIONS = [Position.up_left, Position.up_right, Position.down_right, Position.down_left]
KNIGHT_DIRECTIONS = [
    Position.up_up_right, Position.up_right_right, Position.down_right_right, Position.down_down_right,
    Position.down_left_left, Position.down_down_left, Position.up_left_left, Position.up_up_left
]

class Turn(Flag):
    WHITE = True
    BLACK = False

DEFAULT = {
    "white_pieces": [
        sum([1 << i for i in range(48, 56)]),   # white pawns
        (1 << 57) + (1 << 62),                  # white knights
        (1 << 58) + (1 << 61),                  # white bishops
        (1 << 56) + (1 << 63),                  # white rooks
        (1 << 59),                              # white queens
        (1 << 60),                              # white kings
    ],
    "black_pieces": [
        sum([1 << i for i in range(8, 16)]),    # black pawns
        (1 << 1) + (1 << 6),                    # black knights
        (1 << 2) + (1 << 5),                    # black bishops
        (1 << 0) + (1 << 7),                    # black rooks
        (1 << 3),                               # black queens
        (1 << 4),                               # black kings
    ],
    "K": (1 << 60) + (1 << 61) + (1 << 62),
    "Q": (1 << 60) + (1 << 59) + (1 << 58),
    "k": (1 << 4) + (1 << 5) + (1 << 6),
    "q": (1 << 4) + (1 << 3) + (1 << 2),
    "turn": Turn.WHITE,

}

class Board:
    def __init__(self, white_pieces=DEFAULT['white_pieces'], black_pieces=DEFAULT['black_pieces'],
            K=DEFAULT['K'], Q=DEFAULT['Q'], k=DEFAULT['K'], q=DEFAULT['q'], turn=DEFAULT['turn']):
        # piece bitboards
        self.white_pieces = white_pieces
        self.black_pieces = black_pieces
        self.pieces_list = [self.white_pieces, self.black_pieces]

        # castling rights bitboards
        self.K = K
        self.Q = Q
        self.k = k
        self.q = q
        self.castle_king = 0
        self.castle_queen = 0

        self.pieces_mask = 0
        self.white_mask = 0
        self.black_mask = 0
        self.turn = turn
        self.enemy = None
        self.all_moves = dict()
        self.en_passant = None
        self.attackers = []

        self.CALC_CONTROLLED_FNCS = [
            self._calc_pawn_controlled, 
            self._calc_knight_controlled, 
            self._calc_bishop_controlled, 
            self._calc_rook_controlled, 
            self._calc_queen_controlled,
            self._calc_king_controlled,
        ]

        self.CALC_MOVES_FNCS = [
            self._calc_pawn_moves, 
            self._calc_knight_moves,  
            self._calc_bishop_moves,
            self._calc_rook_moves,
            self._calc_queen_moves,
            self._calc_king_moves,
        ]

        self.updated = False
        self._ensure_updated()

    def __str__(self) -> str:
        self._ensure_updated()

        position = Position()
        ret = ""
        for i in range(8):
            ret += "  " + "-" * 33 + "\n"
            ret += str(8 - i) + " |"
            for j in range(8):
                ret += " " + self._print_piece(position) + " |"
                position = position.next()
            ret += "\n"
        ret += "  " + "-" * 33 + "\n"
        ret += "    a   b   c   d   e   f   g   h\n"
        ret += f"""{"White" if self.turn == Turn.WHITE else "Black"} to move"""
        return ret

    def is_piece(self, position: Position) -> bool:
        self._ensure_updated()
        return position.overlap(self.pieces_mask) > 0

    def is_enemy_controlled(self, position: Position) -> bool:
        self._ensure_updated()
        return position.overlap(self.controlled) > 0

    def get_piece(self, position: Position) -> Piece:
        self._ensure_updated()
        if not (position.overlap(self.pieces_mask)):
            return Piece.EMPTY
        
        if position.overlap(self.white_mask):
            for i, p in enumerate(self.white_pieces):
                if position.overlap(p):
                    return Piece(i)
        elif position.overlap(self.black_mask):
            for i, p in enumerate(self.black_pieces):
                if position.overlap(p):
                    return Piece(i + COLOR_DELIMITER)

    def _print_piece(self, position: Position) -> str:
        self._ensure_updated()
        representations = ['p', 'n', 'b', 'r', 'q', 'k', ' ']
        piece = self.get_piece(position)
        piece_repr = piece % COLOR_DELIMITER if piece != Piece.EMPTY else len(representations) - 1
        return representations[piece_repr]

    def _ensure_updated(self) -> None:
        if self.updated:
            return
        
        self.white_mask = 0
        for p in self.white_pieces:
            self.white_mask |= p
        self.black_mask = 0
        for p in self.black_pieces:
            self.black_mask |= p
        self.pieces_mask = self.black_mask | self.white_mask

        if self.turn == Turn.WHITE:
            self.enemy = self.black_mask
            self.enemy_list = self.black_pieces
            self.friendly_list = self.white_pieces
            self.castle_king = self.K
            self.castle_queen = self.Q
        elif self.turn == Turn.BLACK:
            self.enemy = self.white_mask
            self.enemy_list = self.white_pieces
            self.friendly_list = self.black_pieces
            self.castle_king = self.k
            self.castle_queen = self.q

        self.in_check = False
        self.calculate_all_valid_moves()

        self.updated = True

    def _get_stop_check(self, king: Position, attacker: Position) -> int:
        if attacker.overlap(self.white_pieces[Piece.KNIGHT] | self.black_pieces[Piece.KNIGHT]):
            return 0
        # elif attacker.overlap(self.white_pieces[Piece.ROOK] | self.black_pieces[Piece.ROOK]):
        # elif attacker.overlap(self.white_pieces[Piece.BISHOP] | self.black_pieces[Piece.BISHOP]):
        diff = king.num - attacker.num
        sign = 1 if diff > 0 else -1
        step = None
        if attacker.num // 8 == king.num // 8:
            step = 1 * sign
        elif (king.num - attacker.num) % 8 == 0:
            step = 8 * sign
        elif (king.num - attacker.num) % 7 == 0:
            step = 7 * sign
        elif (king.num - attacker.num) % 9 == 0:
            step = 9 * sign
        
        nums = range(attacker.num, king.num, step)
        print(attacker.num, king.num)
        print([Position(n) for n in nums])
        return sum([1 << n for n in nums])

    def calculate_all_valid_moves(self) -> dict[Position, dict[Position, Move]]:
        if self.updated:
            return self.all_moves
        
        self.attackers = []
        # need to iterate through enemy pieces first to get controlled squares for king
        self.controlled = 0
        for p, fnc in zip(self.enemy_list, self.CALC_CONTROLLED_FNCS):
            pieces = p
            while pieces > 0:
                fnc(Position(ctzll(pieces)))
                pieces &= pieces - 1

        # now, iterate through own pieces to get valid moves
        self.all_moves = dict()
        for p, fnc in zip(self.friendly_list, self.CALC_MOVES_FNCS):
            pieces = p
            while pieces > 0:
                start = Position(ctzll(pieces))
                self.all_moves[start] = fnc(start)
                pieces &= pieces - 1

        # not optimal, but if in check, filter out moves
        if len(self.attackers) == 1:
            print("single check")
            filtered_moves = dict()

            king_position = Position(ctzll(self.friendly_list[Piece.KING]))
            stop_check = self._get_stop_check(king_position, self.attackers[0])
            for s, moves in self.all_moves.items():
                if s == king_position:
                    filtered_moves[king_position] = self.all_moves[king_position]
                    continue
                filtered_moves[s] = dict()
                for e, mv in moves.items():
                    if e.overlap(stop_check):
                        filtered_moves[s][e] = mv
            for k, v in self.all_moves.items():
                if k == Position(ctzll(self.friendly_list[Piece.KING])):
                    filtered_moves[k] = v
            self.all_moves = filtered_moves

        elif len(self.attackers) >= 2: 
            print("double check")
            king_position = Position(ctzll(self.friendly_list[Piece.KING]))
            self.all_moves = {king_position: self.all_moves[king_position]}
        return self.all_moves

    def get_valid_moves(self, position: Position) -> dict[Position, Move]:
        self._ensure_updated()
        if position in self.all_moves:
            return self.all_moves[position]
        return None

    def _calc_pawn_controlled(self, position: Position) -> None:
        if position.overlap(self.white_mask):
            pawn_take_left = Position.up_left
            pawn_take_right = Position.up_right
        elif position.overlap(self.black_mask):
            pawn_take_left = Position.down_right
            pawn_take_right = Position.down_left

        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take:
                if take.overlap(self.friendly_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= take.pos

    def _calc_pawn_moves(self, position: Position) -> dict[Position, Move]:
        if position.overlap(self.white_mask):
            pawn_forward = Position.up
            pawn_take_left = Position.up_left
            pawn_take_right = Position.up_right
        elif position.overlap(self.black_mask):
            pawn_forward = Position.down
            pawn_take_left = Position.down_right
            pawn_take_right = Position.down_left
        moves = dict()
        # calculate forward pawn moves
        forward = pawn_forward(position)
        if forward and not forward.overlap(self.pieces_mask):
            moves[forward] = Move(position, forward, MoveType.MOVE, Piece.PAWN)
            fforward = pawn_forward(forward)
            if ((self.turn == Turn.WHITE and position.rank() == 2) or (self.turn == Turn.BLACK and position.rank() == 7)) \
                    and fforward and not fforward.overlap(self.pieces_mask):
                moves[fforward] = Move(position, fforward, MoveType.PAWN_TWO_MOVE, Piece.PAWN)
        # calculate taking on left and right
        for pawn_take in [pawn_take_left, pawn_take_right]:
            take = pawn_take(position)
            if take is None:
                continue
            if take.overlap(self.enemy):
                moves[take] = Move(position, take, MoveType.TAKE, Piece.PAWN)
            elif self.en_passant and take == self.en_passant:
                moves[take] = Move(position, take, MoveType.EN_PASSANT, Piece.PAWN)
        return moves

    def _calc_knight_controlled(self, position: Position) -> None:
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv:
                if mv.overlap(self.friendly_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos

    def _calc_knight_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        for dir in KNIGHT_DIRECTIONS:
            mv = dir(position)
            if mv is None:
                continue
            # calculate unoccupied squares
            if not mv.overlap(self.pieces_mask):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.KNIGHT)
            # calculate taking enemy pieces
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.KNIGHT)
        return moves

    def _calc_bishop_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for bishop piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friendly_list[Piece.KING])):
                if mv.overlap(self.friendly_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_bishop_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each diagonal direction
        for dir in BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.BISHOP)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.BISHOP)
        return moves
    
    def _calc_rook_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            # need to account for rook piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friendly_list[Piece.KING])):
                if mv.overlap(self.friendly_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_rook_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each straight direction
        for dir in ROOK_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.ROOK)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.ROOK)
        return moves

    def _calc_queen_controlled(self, position: Position) -> None:
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            # need to account for queen piercing through the king
            while mv and (not mv.overlap(self.pieces_mask) or mv.overlap(self.friendly_list[Piece.KING])):
                if mv.overlap(self.friendly_list[Piece.KING]):
                    self.attackers.append(position)
                self.controlled |= mv.pos
                mv = dir(mv)
            if mv:
                self.controlled |= mv.pos

    def _calc_queen_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        # calculate moves in each diagonal direction
        for dir in ROOK_DIRECTIONS + BISHOP_DIRECTIONS:
            mv = dir(position)
            while mv and not mv.overlap(self.pieces_mask):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.QUEEN)
                mv = dir(mv)
            # if there's an enemy piece at the end, then we could take
            if mv and mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.QUEEN)
        return moves

    def _calc_king_controlled(self, position: Position) -> None:
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv:
                self.controlled |= mv.pos

    def _calc_king_moves(self, position: Position) -> dict[Position, Move]:
        moves = dict()
        for dir in BISHOP_DIRECTIONS + ROOK_DIRECTIONS:
            mv = dir(position)
            if mv is None or mv.overlap(self.controlled):
                continue
            if not mv.overlap(self.pieces_mask):
                moves[mv] = Move(position, mv, MoveType.MOVE, Piece.KING)
            elif mv.overlap(self.enemy):
                moves[mv] = Move(position, mv, MoveType.TAKE, Piece.KING)
        # check for castling
        if self.castle_king and (self.castle_king & (self.pieces_mask | self.controlled)) == 0:
            castle_kingside = position.right().right()
            moves[castle_kingside] = Move(position, castle_kingside, MoveType.CASTLE_KING, Piece.KING)
        if self.castle_queen and (self.castle_queen & (self.pieces_mask | self.controlled)) == 0:
            castle_queenside = position.left().left()
            moves[castle_queenside] = Move(position, castle_queenside, MoveType.CASTLE_QUEEN, Piece.KING)
        return moves

    def make_move(self, s: Position, e: Position):
        self._ensure_updated()
        if e in self.all_moves[s]:
            move = self.all_moves[s][e]

            # erase taken square
            if move.type == MoveType.TAKE: 
                for i in range(len(self.white_pieces)):
                    self.white_pieces[i] &= ~e.pos
                for i in range(len(self.black_pieces)):
                    self.black_pieces[i] &= ~e.pos
            # erase pawn that was en passanted
            elif move.type == MoveType.EN_PASSANT:
                taken_pawn = e.down() if self.turn == Turn.WHITE else e.up()
                self.white_pieces[Piece.PAWN] &= ~taken_pawn.pos
                self.black_pieces[Piece.PAWN] &= ~taken_pawn.pos
            # move the castled rook
            elif move.type == MoveType.CASTLE_KING:
                self.pieces_list[1 - self.turn.value][Piece.ROOK] &= ~e.right().pos
                self.pieces_list[1 - self.turn.value][Piece.ROOK] |= e.left().pos
            # move the castled rook
            elif move.type == MoveType.CASTLE_QUEEN:
                self.pieces_list[1 - self.turn.value][Piece.ROOK] &= ~e.left().left().pos
                self.pieces_list[1 - self.turn.value][Piece.ROOK] |= e.right().pos
            
            # if a pawn was moved two squares, then allow for en_passant
            if move.type == MoveType.PAWN_TWO_MOVE:
                self.en_passant = e.down() if self.turn == Turn.WHITE else e.up()
            else:
                self.en_passant = None
            
            # if king was moved, then disable castling rights
            if s.overlap(self.white_pieces[Piece.KING] | self.black_pieces[Piece.KING]):
                if self.turn == Turn.WHITE:
                    self.K = 0
                    self.Q = 0
                elif self.turn == Turn.BLACK:
                    self.k = 0
                    self.q = 0

            # make the move of original piece
            self.pieces_list[1 - self.turn.value][move.piece] &= ~s.pos
            self.pieces_list[1 - self.turn.value][move.piece] |= e.pos

            self.turn = ~self.turn
            self.updated = False
        else:
            ex = Move(s, e)
            print(f"{ex} is not a move!")

if __name__=="__main__":
    board = Board()
    print(board)
    board.make_move(Position.get_position("e2"), Position.get_position("e4"))
    print(board)
    # board.make_move(Position.get_position("e7"), Position.get_position("e5"))
    # print(board)
    # board.make_move(Position.get_position("d2"), Position.get_position("d4"))
    # print(board)
    # board.make_move(Position.get_position("e5"), Position.get_position("d4"))
    # print(board)
    # print(board.get_valid_moves(Position.get_position("d1")))
    # board.make_move(Position.get_position("d1"), Position.get_position("d4"))
    # print(board)
    # board.make_move(Position.get_position("e8"), Position.get_position("e7"))
    # print(board)
    # board.make_move(Position.get_position("e1"), Position.get_position("e2"))
    # print(board)
    # board.make_move(Position.get_position("e7"), Position.get_position("d6"))
    # print(board)
