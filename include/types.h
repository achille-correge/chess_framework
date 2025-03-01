#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_SEARCH_PLY 128
#define MAX_MOVES 128

typedef int Score;

typedef uint64_t Bitboard;

typedef enum
{
    WHITE,
    BLACK
} Color;

typedef enum
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
} PieceType;

typedef struct
{
    char name;  // name of the piece
    char color; // color of the piece ('w' for white, 'b' for black, ' ' for empty)
} Piece;

typedef struct
{
    int x;
    int y;
} Coords;

typedef struct
{
    Coords init_co;
    Coords dest_co;
    char promotion;
} Move;

typedef struct
{
    Piece board[8][8];
    bool white_kingside_castlable; // true if white can castle
    bool white_queenside_castlable;
    bool black_kingside_castlable;
    bool black_queenside_castlable;
    int black_pawn_passant; // -1 if no pawn can be taken en passant, otherwise the column of the pawn
    int white_pawn_passant;
    int fifty_move_rule;
    Color player;
} BoardState;

typedef struct position_list
{
    BoardState *board_s;
    struct position_list *tail;
} PositionList;

typedef struct
{
    Move moves[MAX_MOVES];
    int size;
} MoveList;

#endif