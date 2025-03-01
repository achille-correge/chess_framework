#include <stdio.h>
#include "types.h"
#include "chess_logic.h"

void print_move(Move move)
{
    char start_col = 'a' + move.init_co.y;
    char start_row = '1' + (move.init_co.x);
    char end_col = 'a' + move.dest_co.y;
    char end_row = '1' + (move.dest_co.x);
    fprintf(stderr, "%c%c -> %c%c\n", start_col, start_row, end_col, end_row);
}

void print_board(BoardState *board_s, FILE *file)
{
    Piece(*board)[8] = board_s->board;
    for (int i = 7; i >= 0; i--)
    {
        fprintf(file, "%d ", i);
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j].color == 'w')
            {
                fprintf(file, "%c ", board[i][j].name);
            }
            else if (board[i][j].color == 'b')
            {
                fprintf(file, "%c ", board[i][j].name + 32);
            }
            else
            {
                fprintf(file, "  ");
            }
        }
        fprintf(file, "\n");
    }
    fprintf(file, "  a b c d e f g h\n");
}

void print_position_list(PositionList *pos_l)
{
    PositionList *current = pos_l;
    int i = 0;
    while (current != NULL)
    {
        fprintf(stderr, "Position %d\n", i);
        print_board(current->board_s, stderr);
        current = current->tail;
        i++;
    }
}