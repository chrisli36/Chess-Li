import pygame
from board import Board
from position import *
from moves import *
from utils import *

pygame.init()

gray = pygame.Color(150, 150, 150)
tan = pygame.Color(210, 180, 140)
dark_tan = pygame.Color(126, 108, 84)
brown = pygame.Color(140, 70, 20)
dark_brown = pygame.Color(84, 42, 12)

display = pygame.display.set_mode((800, 800))
pygame.display.set_caption("Chess Li")

square_size = 100
pieces_image = pygame.image.load("pieces.png")
pieces_image = pygame.transform.scale(pieces_image, (600, 200))

piece_sprites = [None] * 12
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
    piece_sprites[p] = pieces_image.subsurface(
        ((i % COLOR_DELIMITER) * square_size, square_size * (i // COLOR_DELIMITER), square_size, square_size))

display = pygame.display.set_mode((800, 800))
pygame.display.set_caption("Chess Li")

board = Board()
running = True
curr_pos = None
prev_pos = None
just_moved = None
possible_moves = None

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            mouse_x, mouse_y = pygame.mouse.get_pos()
            file = mouse_x // square_size
            rank = mouse_y // square_size
            curr_pos = Position(rank * 8 + file)
            # print(f"Selected {curr_pos}")

            if prev_pos:
                moves = board.get_moves(prev_pos)
                if curr_pos == prev_pos:
                    prev_pos = None
                elif moves is None:
                    if board.is_piece(curr_pos):
                        prev_pos = curr_pos
                    else:
                        prev_pos = None
                elif curr_pos.overlap(moves.mask):
                    # if board.is_promoting()
                    board.make_move(prev_pos, curr_pos, moves.piece)
                    just_moved = (prev_pos, curr_pos)
                    prev_pos = None
                elif board.is_piece(curr_pos):
                    prev_pos = curr_pos
                else:
                    prev_pos = None
            elif board.is_piece(curr_pos):
                prev_pos = curr_pos

            possible_moves = board.get_moves(prev_pos) if prev_pos else None

    display.fill((255, 255, 255))
    position = Position()
    for rank in range(8):
        for file in range(8):
            color = tan if (rank + file) % 2 == 0 else brown

            # draw previous position
            if (prev_pos and position == prev_pos) or (just_moved and position in just_moved):
                pygame.draw.rect(
                    display,
                    dark_tan if (rank + file) % 2 == 0 else dark_brown,
                    (file * square_size, rank * square_size, square_size, square_size),
                )
            else:
                pygame.draw.rect(display, color, (file * square_size, rank * square_size, square_size, square_size))

            # draw pieces on the board
            piece = board.get_piece(position)
            if piece != Piece.EMPTY:
                display.blit(piece_sprites[piece], (file * square_size, rank * square_size))

            if possible_moves and position.overlap(possible_moves.mask):
                if board.is_piece(position):
                    pygame.draw.circle(
                        display,
                        gray,
                        (file * square_size + square_size // 2, rank * square_size + square_size // 2), 
                        square_size // 2,
                        10,
                    )
                else:
                    pygame.draw.circle(
                        display, 
                        gray, # gray_tan if (rank + file) % 2 == 0 else gray_brown, 
                        (file * square_size + square_size // 2, rank * square_size + square_size // 2), 
                        square_size // 6,
                    )
            
            if board.is_enemy_controlled(position):
                pygame.draw.rect(
                    display,
                    (255, 0, 0),
                    (file * square_size, rank * square_size, square_size, square_size),
                    3,
                )

            position = position.next()

    pygame.display.flip()

pygame.quit()
