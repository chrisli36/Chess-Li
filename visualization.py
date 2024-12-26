import pygame
from board import Board

pygame.init()

white, black, red = (255, 255, 2555), (0, 0, 0), (255, 0, 0)

display = pygame.display.set_mode((800, 600))
pygame.display.set_caption("Chess Li")

size = 20
display.fill(white)
for i in range(1, 9):
    for z in range(1, 9):
        if i + z % 2 == 0:
            pass
        else:
            pass