#ifndef LEGAL_MOVES_H
#define LEGAL_MOVES_H

#include <stdbool.h>
#include "types.h"

bool can_move_pawn(BoardState *board_s, Piece selected_piece, Coords init_co, Coords dest_co);
bool can_move_knight(BoardState *board_s, Coords init_co, Coords dest_co);
bool can_move_rook(BoardState *board_s, Coords init_co, Coords dest_co);
bool can_move_bishop(BoardState *board_s, Coords init_co, Coords dest_co);
bool can_move_queen(BoardState *board_s, Coords init_co, Coords dest_co);
bool can_move_king(BoardState *board_s, Piece selected_piece, Coords init_co, Coords dest_co);

#endif