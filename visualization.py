import pygame
from board import Board, PROMOTIONS, GameState
from engine import Engine
from position import *
from moves import *
from utils import *
import threading

GRAY = pygame.Color(150, 150, 150)
TAN = pygame.Color(210, 180, 140)
DARK_TAN = pygame.Color(126, 108, 84)
BROWN = pygame.Color(140, 70, 20)
DARK_BROWN = pygame.Color(84, 42, 12)
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
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

class Visualization:
    def __init__(self, game_type):
        pygame.init()

        self.display = pygame.display.set_mode((800, 800))
        self.square_size = 100
        pygame.display.set_caption("Chess Li")
        self.FONT = pygame.font.Font(None, 36)

        self._load_pieces()

        self.game_type = game_type
        self.board = Board()
        self.engine = Engine()

    def _load_pieces(self):
        pieces_image = pygame.image.load("pieces.png")
        pieces_image = pygame.transform.scale(pieces_image, (600, 200))
        self.piece_sprites = [None] * 12
        piece_to_idx = {
            Piece.WHITE_PAWN: 5,
            Piece.WHITE_KNIGHT: 3,
            Piece.WHITE_BISHOP: 2,
            Piece.WHITE_ROOK: 4,
            Piece.WHITE_QUEEN: 1,
            Piece.WHITE_KING: 0,
            Piece.BLACK_PAWN: 11,
            Piece.BLACK_KNIGHT: 9,
            Piece.BLACK_BISHOP: 8,
            Piece.BLACK_ROOK: 10,
            Piece.BLACK_QUEEN: 7,
            Piece.BLACK_KING: 6,
        }
        for p in PIECES:
            i = piece_to_idx[p]
            self.piece_sprites[p] = pieces_image.subsurface(
                ((i % COLOR_DELIMITER) * self.square_size, 
                 self.square_size * (i // COLOR_DELIMITER), 
                 self.square_size, 
                 self.square_size))

    def draw_potential_capture(self, file, rank):
        pygame.draw.circle(
            self.display,
            GRAY,
            (file * self.square_size + self.square_size // 2, rank * self.square_size + self.square_size // 2), 
            self.square_size // 2,
            10,
        )

    def draw_potential_move(self, file, rank):
        pygame.draw.circle(
            self.display, 
            GRAY, # gray_tan if (rank + file) % 2 == 0 else gray_brown, 
            (file * self.square_size + self.square_size // 2, rank * self.square_size + self.square_size // 2), 
            self.square_size // 6,
        )

    def draw_square(self, file, rank, color):
        pygame.draw.rect(
            self.display,
            color,
            (file * self.square_size, rank * self.square_size, self.square_size, self.square_size),
        )

    def highlight_square(self, file, rank):
        pygame.draw.rect(
            self.display,
            (255, 0, 0),
            (file * self.square_size, rank * self.square_size, self.square_size, self.square_size),
            3,
        )

    def draw_piece(self, file, rank, piece: Piece):
        if piece != Piece.EMPTY:
            self.display.blit(self.piece_sprites[piece], (file * self.square_size, rank * self.square_size))

    def run(self):
        self.bot_thinking = False if self.game_type & 1 else True
        if self.bot_thinking:
            threading.Thread(target=self.bot_move).start()

        # board variables
        self.promotion_flag = False
        self.curr_sel = None
        self.prev_sel = None
        self.moves_stack = list()
        self.moves = None

        self.running = True
        while self.running:
            for event in pygame.event.get():
                # check if user quits the application
                if event.type == pygame.QUIT:
                    self.running = False
                # we're waiting on the bot to make a move
                elif self.bot_thinking:
                    continue
                elif self.game_type == 0:
                    threading.Thread(target=self.bot_move).start()
                # if the user is promoting, then the user can only press 1-4
                elif self.promotion_flag:
                    if event.type == pygame.KEYDOWN and event.unicode in ['1', '2', '3', '4']:
                        self.user_move(int(event.unicode))
                # otherwise, allow normal user interaction
                else:
                    if event.type == pygame.MOUSEBUTTONDOWN:
                        mouse_x, mouse_y = pygame.mouse.get_pos()
                        file, rank = mouse_x // self.square_size, mouse_y // self.square_size
                        self.curr_sel = Position(rank * 8 + file)

                        if self.prev_sel: 
                            self.moves = self.board.get_moves(self.prev_sel)
                            if self.curr_sel == self.prev_sel:
                                self.prev_sel = None
                            elif self.moves is None:
                                self.prev_sel = self.curr_sel if self.board.is_piece(self.curr_sel) else None
                            elif self.curr_sel.overlap(self.moves.mask):
                                if self.board.is_promoting(self.prev_sel, self.curr_sel):
                                    self.promotion_flag = True
                                else:
                                    self.user_move()
                            elif self.board.is_piece(self.curr_sel):
                                self.prev_sel = self.curr_sel
                            else:
                                self.prev_sel = None
                        elif self.board.is_piece(self.curr_sel):
                            self.prev_sel = self.curr_sel
                        
                        # if we're pawn promoting, we don't want to update self.moves until promotion
                        if not self.promotion_flag:
                            self.moves = self.board.get_moves(self.prev_sel) if self.prev_sel else None
                    elif event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_LEFT and self.moves_stack:
                            self.user_undo()
            if self.bot_thinking:
                self.display.blit(self.cached_surface, (0, 0))
                pygame.display.flip()
            elif self.promotion_flag:
                self.draw_board()
                rendered_text = self.FONT.render("Choose promotion 1-4: queen, rook, bishop, knight", True, WHITE)
                self.display.blit(rendered_text, (0, 300))
                pygame.display.flip()
            else:
                self.draw_board()
                pygame.display.flip()
        pygame.quit()

    def user_move(self, promo=None):
        if self.promotion_flag:
            taken, sc = self.board.make_move(self.prev_sel, self.curr_sel, self.moves.piece, PROMOTIONS[promo - 1])
            self.moves_stack.append(((self.prev_sel, self.curr_sel), self.moves.piece, taken, sc))
            self.promotion_flag = False
            self.prev_sel = None
            self.moves = None
        else:
            taken, sc = self.board.make_move(self.prev_sel, self.curr_sel, self.moves.piece)
            self.moves_stack.append(((self.prev_sel, self.curr_sel), self.moves.piece, taken, sc))
            self.prev_sel = None
        # if the user is playing against the bot, then the bot should make a move
        if self.game_type == 1 or self.game_type == 2:
            threading.Thread(target=self.bot_move).start()

        self.check_game_over()

    def check_game_over(self):
        self.board._ensure_updated()
        if self.board.game_state != GameState.PLAYING:
            if self.board.game_state == GameState.WHITE:
                print("WHITE WON!")
            elif self.board.game_state == GameState.BLACK:
                print("BLACK WON!")
            elif self.board.game_state == GameState.DRAW:
                print("DRAW!")
            else:
                print("Game state error")
            return True
        return False

    def user_undo(self):
        (s, e), p, t, sc = self.moves_stack.pop(-1)
        self.board.undo_move(s, e, p, t, sc)
        self.curr_sel = None
        self.prev_sel = None

        # if it's a user vs bot, then undo the bot move as well
        if self.game_type == 1 or self.game_type == 2:
            (s, e), p, t, sc = self.moves_stack.pop(-1)
            self.board.undo_move(s, e, p, t, sc)

    def bot_move(self):
        self.draw_board()
        pygame.display.flip()
        self.cached_surface = self.display.copy()
        self.bot_thinking = True
        if self.check_game_over():
            return

        alpha, beta = float('-inf'), float('inf')
        best_move = None
        all_moves = self.board.get_all_moves_list()
        all_moves.sort(key=lambda x: self.engine.score_move(self.board, x[2], x[1], x[3]),reverse=True)
        self.engine.positions = 0
        for s, e, piece, promo in all_moves:
            taken, state_changes = self.board.make_move(s, e, piece, promo)
            curr_eval = -self.engine.minimax(self.board, 3, -beta, -alpha)
            if best_move is None or curr_eval > best_move[0]:
                best_move = (curr_eval, s, e, piece, promo)
            self.board.undo_move(s, e, piece, taken, state_changes)

            alpha = max(alpha, curr_eval)

        # make the move
        s, e, piece, promo = best_move[1:]
        out = f"LI-BOT: evaluated {self.engine.positions} positions. {piece} from {s} to {e}"
        out += f" {promo}" if promo else ""
        print(out)
        taken, sc = self.board.make_move(s, e, piece, promo)
        self.moves_stack.append(((s, e), piece, taken, sc))

        self.check_game_over()
        self.bot_thinking = False

    def draw_board(self):
        self.display.fill((255, 255, 255))
        position = Position()
        for rank in range(8):
            for file in range(8):
                # draw previous position
                if (self.prev_sel and position == self.prev_sel) or (self.moves_stack and position in self.moves_stack[-1][0]):
                    self.draw_square(file, rank, DARK_TAN if (rank + file) % 2 == 0 else DARK_BROWN)
                else:
                    self.draw_square(file, rank, TAN if (rank + file) % 2 == 0 else BROWN)

                # draw pieces on the board
                piece = self.board.get_piece(position)
                self.draw_piece(file, rank, piece)

                # draw potential moves
                if self.moves and position.overlap(self.moves.mask):
                    if self.board.is_piece(position):
                        self.draw_potential_capture(file, rank)
                    else:
                        self.draw_potential_move(file, rank)
                    
                # # debugging: highlight enemy controlled
                # if self.board.is_enemy_controlled(position):
                #     self.highlight_square(file, rank)

                position = position.next()

if __name__ == "__main__":
    vis = Visualization(game_type=1)
    vis.run()