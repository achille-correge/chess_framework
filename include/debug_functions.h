#ifndef DEBUG_FUNCTIONS_H
#define DEBUG_FUNCTIONS_H

#include <stdio.h>
#include "types.h"

void print_move(Move move);
void print_board(BoardState *board_s, FILE *file);
void print_position_list(PositionList *pos_l);

#endif