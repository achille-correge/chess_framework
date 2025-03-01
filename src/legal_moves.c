#include <stdbool.h>
#include <stdlib.h>
#include "types.h"
#include "legal_moves.h"
#include "chess_logic.h"

#include <stdio.h>

bool can_move_pawn(BoardState *board_s, Piece selected_piece, Coords init_co, Coords dest_co)
{
    // fprintf(stderr, "cmp: piece is %c %c\n", selected_piece.name, selected_piece.color);
    Piece(*board)[8] = board_s->board;
    int newx = dest_co.x;
    int newy = dest_co.y;
    if (selected_piece.color == 'w')
    {
        // check if the pawn is moving forward
        if (newx == init_co.x + 1 && newy == init_co.y && is_empty(board[newx][newy]))
        {
            return true;
        }
        // check if the pawn is moving diagonally to take a piece
        else if (newx == init_co.x + 1 && abs(newy - init_co.y) == 1 && board[newx][newy].name != ' ')
        {
            return true;
        }
        // check if the pawn is moving two squares forward
        else if (init_co.x == 1 && newx == 3 && newy == init_co.y && board[2][newy].name == ' ' && is_empty(board[newx][newy]))
        {
            return true;
        }
        // check for en passant
        else if (init_co.x == 4 && newx == init_co.x + 1 && abs(newy - init_co.y) == 1 && board_s->black_pawn_passant == newy)
        {
            return true;
        }
    }
    else
    {
        if (newx == init_co.x - 1 && newy == init_co.y && is_empty(board[newx][newy]))
        {
            return true;
        }
        else if (newx == init_co.x - 1 && abs(newy - init_co.y) == 1 && board[newx][newy].name != ' ')
        {
            return true;
        }
        else if (init_co.x == 6 && newx == 4 && newy == init_co.y && board[5][newy].name == ' ' && is_empty(board[newx][newy]))
        {
            return true;
        }
        else if (init_co.x == 3 && newx == init_co.x - 1 && abs(newy - init_co.y) == 1 && board_s->white_pawn_passant == newy)
        {
            return true;
        }
    }
    return false;
}

bool can_move_rook(BoardState *board_s, Coords init_co, Coords dest_co)
{
    Piece(*board)[8] = board_s->board;
    int newx = dest_co.x;
    int newy = dest_co.y;
    if (newx == init_co.x)
    {
        if (newy > init_co.y)
        {
            for (int i = init_co.y + 1; i < newy; i++)
            {
                if (board[init_co.x][i].name != ' ')
                {
                    return false;
                }
            }
        }
        else
        {
            for (int i = init_co.y - 1; i > newy; i--)
            {
                if (board[init_co.x][i].name != ' ')
                {
                    return false;
                }
            }
        }
        return true;
    }
    else if (newy == init_co.y)
    {
        if (newx > init_co.x)
        {
            for (int i = init_co.x + 1; i < newx; i++)
            {
                if (board[i][init_co.y].name != ' ')
                {
                    return false;
                }
            }
        }
        else
        {
            for (int i = init_co.x - 1; i > newx; i--)
            {
                if (board[i][init_co.y].name != ' ')
                {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

bool can_move_knight(BoardState *board_s, Coords init_co, Coords dest_co)
{
    int newx = dest_co.x;
    int newy = dest_co.y;
    if ((newx == init_co.x + 2 && newy == init_co.y + 1) ||
        (newx == init_co.x + 2 && newy == init_co.y - 1) ||
        (newx == init_co.x - 2 && newy == init_co.y + 1) ||
        (newx == init_co.x - 2 && newy == init_co.y - 1) ||
        (newx == init_co.x + 1 && newy == init_co.y + 2) ||
        (newx == init_co.x + 1 && newy == init_co.y - 2) ||
        (newx == init_co.x - 1 && newy == init_co.y + 2) ||
        (newx == init_co.x - 1 && newy == init_co.y - 2))
    {
        return true;
    }
    return false;
}
bool can_move_bishop(BoardState *board_s, Coords init_co, Coords dest_co)
{
    int newx = dest_co.x;
    int newy = dest_co.y;
    Piece(*board)[8] = board_s->board;
    if (newx - init_co.x == newy - init_co.y)
    {
        if (newx > init_co.x)
        {
            for (int i = 1; i < newx - init_co.x; i++)
            {
                if (board[init_co.x + i][init_co.y + i].name != ' ')
                {
                    return false;
                }
            }
        }
        else
        {
            for (int i = 1; i < init_co.x - newx; i++)
            {
                if (board[init_co.x - i][init_co.y - i].name != ' ')
                {
                    return false;
                }
            }
        }
        return true;
    }
    else if (newx - init_co.x == init_co.y - newy)
    {
        if (newx > init_co.x)
        {
            for (int i = 1; i < newx - init_co.x; i++)
            {
                if (board[init_co.x + i][init_co.y - i].name != ' ')
                {
                    return false;
                }
            }
        }
        else
        {
            for (int i = 1; i < init_co.x - newx; i++)
            {
                if (board[init_co.x - i][init_co.y + i].name != ' ')
                {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

bool can_move_queen(BoardState *board_s, Coords init_co, Coords dest_co)
{
    return can_move_rook(board_s, init_co, dest_co) || can_move_bishop(board_s, init_co, dest_co);
}

bool can_move_king(BoardState *board_s, Piece selected_piece, Coords init_co, Coords dest_co)
{
    int newx = dest_co.x;
    int newy = dest_co.y;
    // fprintf(stderr, "can_move_king: %c (%d, %d) -> (%d, %d) castling rights: %d %d\n", selected_piece.color, init_co.x, init_co.y, newx, newy, board_s->white_kingside_castlable, board_s->white_queenside_castlable);

    if (abs(newx - init_co.x) <= 1 && abs(newy - init_co.y) <= 1)
    {
        return true;
    }

    // castling
    // white kingside castling
    else if (selected_piece.color == 'w' && newx == 0 && newy == 6 && board_s->white_kingside_castlable)
    {
        if (board_s->board[0][5].name == ' ' && board_s->board[0][6].name == ' ' && board_s->board[0][7].name == 'R' && board_s->board[0][7].color == 'w' && !is_attacked(board_s, (Coords){0, 5}, 'w', false) && !is_attacked(board_s, (Coords){0, 6}, 'w', false) && !is_attacked(board_s, (Coords){0, 4}, 'w', false))
        {
            return true;
        }
    }
    // white queenside castling
    else if (selected_piece.color == 'w' && newx == 0 && newy == 2 && board_s->white_queenside_castlable)
    {
        if (board_s->board[0][1].name == ' ' && board_s->board[0][2].name == ' ' && board_s->board[0][3].name == ' ' && board_s->board[0][0].name == 'R' && board_s->board[0][0].color == 'w' && !is_attacked(board_s, (Coords){0, 2}, 'w', false) && !is_attacked(board_s, (Coords){0, 3}, 'w', false) && !is_attacked(board_s, (Coords){0, 4}, 'w', false))
        {
            return true;
        }
    }
    // black kingside castling
    else if (selected_piece.color == 'b' && newx == 7 && newy == 6 && board_s->black_kingside_castlable)
    {
        if (board_s->board[7][5].name == ' ' && board_s->board[7][6].name == ' ' && board_s->board[7][7].name == 'R' && board_s->board[7][7].color == 'b' && !is_attacked(board_s, (Coords){7, 5}, 'b', false) && !is_attacked(board_s, (Coords){7, 6}, 'b', false) && !is_attacked(board_s, (Coords){7, 4}, 'b', false))
        {
            return true;
        }
    }
    // black queenside castling
    else if (selected_piece.color == 'b' && newx == 7 && newy == 2 && board_s->black_queenside_castlable)
    {
        if (board_s->board[7][1].name == ' ' && board_s->board[7][2].name == ' ' && board_s->board[7][3].name == ' ' && board_s->board[7][0].name == 'R' && board_s->board[7][0].color == 'b' && !is_attacked(board_s, (Coords){7, 2}, 'b', false) && !is_attacked(board_s, (Coords){7, 3}, 'b', false) && !is_attacked(board_s, (Coords){7, 4}, 'b', false))
        {
            return true;
        }
    }
    return false;
}