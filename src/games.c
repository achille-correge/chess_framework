#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "games.h"
#include "communication.h"
#include "types.h"
#include "chess_logic.h"
#include "debug_functions.h"
#include "starting_pos_fen.h"

#define M_PI 3.14159265358979323846
#define MAX_MSG_LENGTH 32000
#define OUTPUT_FILE stderr

bool handle_uci_command(char *token)
{
    while (token != NULL)
    {
        if (strcmp(token, "uciok") == 0)
        {
            return true;
        }
        token = strtok(NULL, "\n");
    }
    return false;
}

Move get_next_move(int pipe_main_to_child[2], int pipe_child_to_main[2], char moves_history[MAX_MSG_LENGTH], char starting_pos[128], int wtime, int btime)
{
    char answer[256] = {0};
    char message[MAX_MSG_LENGTH];
    if (strlen(moves_history) == 0)
    {
        sprintf(message, "position %s\n", starting_pos);
    }
    else
    {
        sprintf(message, "position %s moves %s\n", starting_pos, moves_history);
    }
    fprintf(OUTPUT_FILE, "Sending command to child: %s\n", message);
    communicate_with_child(pipe_main_to_child, pipe_child_to_main, message, answer, 0);
    char message2[256];
    sprintf(message2, "go wtime %d btime %d\n", wtime, btime);
    fprintf(OUTPUT_FILE, "Sending command to child: %s\n", message2);

    // measure time taken to answer the go command
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    communicate_with_child(pipe_main_to_child, pipe_child_to_main, message2, answer, 1);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    if (elapsed_time > 0.11)
    {
        fprintf(stdout, "Answer took too much time: Elapsed time is %f seconds.\n", elapsed_time);
        exit(EXIT_FAILURE);
    }
    fprintf(OUTPUT_FILE, "Answer: %s\n", answer);
    Move move;
    if (strcmp(answer, "bestmove (none)\n") == 0)
    {
        move.init_co = empty_coords();
        move.dest_co = empty_coords();
        move.promotion = ' ';
    }
    else
    {
        char *token = strtok(answer, " ");
        token = strtok(NULL, " ");
        move.init_co.y = token[0] - 'a';
        move.init_co.x = token[1] - '1';
        move.dest_co.y = token[2] - 'a';
        move.dest_co.x = token[3] - '1';
        if (strlen(token) == 6)
        {
            fprintf(OUTPUT_FILE, "token: %s\n", token);
            move.promotion = token[4];
        }
        else
        {
            move.promotion = ' ';
        }
    }
    return move;
}

void concatenate_moves(char moves_history[MAX_MSG_LENGTH], Move move)
{
    char move_str0[5];
    char move_str[6];
    sprintf(move_str0, "%c%c%c%c", move.init_co.y + 'a', move.init_co.x + '1', move.dest_co.y + 'a', move.dest_co.x + '1');
    if (move.promotion != ' ')
    {
        sprintf(move_str, "%s%c", move_str0, move.promotion);
    }
    else
    {
        sprintf(move_str, "%s", move_str0);
    }
    if (moves_history[0] != '\0')
    {
        strcat(moves_history, " ");
    }
    strcat(moves_history, move_str);
}

PositionList *play_turn(int pipe_main_to_child[2], int pipe_child_to_main[2], char moves_history[MAX_MSG_LENGTH], PositionList *pos_l, char starting_pos[128], int wtime, int btime)
{
    Move move = get_next_move(pipe_main_to_child, pipe_child_to_main, moves_history, starting_pos, wtime, btime);
    if (is_empty_coords(move.init_co))
    {
        fprintf(OUTPUT_FILE, "No moves given\n");
        free_position_list(pos_l);
        return NULL;
    }
    concatenate_moves(moves_history, move);
    fprintf(OUTPUT_FILE, "moves_history: %s\n", moves_history);

    BoardState *new_board_s = malloc(sizeof(BoardState));
    if (new_board_s == NULL)
    {
        free_position_list(pos_l);
        return NULL;
    }
    *new_board_s = *pos_l->board_s;
    new_board_s = move_piece(new_board_s, move);
    PositionList *new_pos_l = save_position(new_board_s, pos_l);
    free(new_board_s);
    new_board_s = new_pos_l->board_s;
    print_board(new_pos_l->board_s, stderr);
    return new_pos_l;
}

// return 1 for victory, 0 for draw, -1 for no end
int check_end(PositionList *pos_l)
{
    if (pos_l == NULL)
    {
        fprintf(stdout, "No moves given (this shouldn't be possinle)\n");
        return 0;
    }
    BoardState *board_s = pos_l->board_s;
    char color = board_s->player == WHITE ? 'w' : 'b';
    if (is_mate(board_s, color))
    {
        if (is_check(board_s, color))
        {
            fprintf(stdout, "Checkmate\n");
            return 1;
        }
        else
        {
            fprintf(stdout, "Stalemate\n");
            return 0;
        }
    }
    else if (threefold_repetition(board_s, pos_l, 0))
    {
        fprintf(stdout, "Threefold repetition\n");
        return 0;
    }
    else if (board_s->fifty_move_rule >= 100)
    {
        fprintf(stdout, "Fifty-move rule\n");
        return 0;
    }
    else if (insufficient_material(board_s))
    {
        fprintf(stdout, "Insufficient material\n");
        return 0;
    }
    return -1;
}

PositionList *load_fen_command(char fen_command[128], PositionList *pos_l)
{
    char FEN[MAX_MSG_LENGTH];
    sscanf(fen_command, "fen \"%[^\"]\"", FEN);
    fprintf(OUTPUT_FILE, "FEN: %s\n", FEN);
    // BoardState *board_s = FEN_to_board(FEN);
    BoardState *board_s = FEN_to_board(FEN);
    if (board_s == NULL)
    {
        free_position_list(pos_l);
        return NULL;
    }
    pos_l = save_position(board_s, pos_l);
    free(board_s);
    return pos_l;
}

// return 1 for white victory, 0 for draw, -1 for white defeat (black victory)
int play_game(int pipe_main_to_child1[2], int pipe_child1_to_main[2], int pipe_main_to_child2[2], int pipe_child2_to_main[2], char starting_pos[128], int wtime, int btime)
{
    char moves_history[MAX_MSG_LENGTH] = "";
    PositionList *pos_l = empty_list();
    if (strcmp(starting_pos, "startpos") == 0)
    {
        pos_l = save_position(init_board(), pos_l);
    }
    else
    {
        pos_l = load_fen_command(starting_pos, pos_l);
    }
    int end;
    while (1)
    {
        pos_l = play_turn(pipe_main_to_child1, pipe_child1_to_main, moves_history, pos_l, starting_pos, wtime, btime);
        if (pos_l == NULL)
        {
            fprintf(stdout, "No moves given / surrend (white)\n");
            return -1;
        }
        end = check_end(pos_l);
        if (end != -1)
        {
            if (pos_l != NULL)
                free_position_list(pos_l);
            return end;
        }

        pos_l = play_turn(pipe_main_to_child2, pipe_child2_to_main, moves_history, pos_l, starting_pos, wtime, btime);
        if (pos_l == NULL)
        {
            fprintf(stdout, "No moves given / surrend (black)\n");
            return 1;
        }
        end = check_end(pos_l);
        if (end != -1)
        {
            if (pos_l != NULL)
                free_position_list(pos_l);
            return -end;
        }
    }
}

// the elo part is inspired by https://3dkingdoms.com/chess/elo.htm source code
double elo_diff(double p)
{
    return -400 * log10(1 / p - 1);
}

double erf_inv(double x)
{
    double a = 8 * (M_PI - 3) / (3 * M_PI * (4 - M_PI));
    double y = log(1 - x * x);
    double z = 2 / (M_PI * a) + y / 2;
    double res = sqrt(sqrt(z * z - y / a) - z);
    if (x >= 0)
        return res;
    else
        return -res;
}

double error_margin(int victories, int defeats, int draws)
{
    double total = victories + defeats + draws;
    double percentage = (victories + 0.5 * draws) / total;
    double wins_dev = victories / total * pow(1 - percentage, 2);
    double loses_dev = defeats / total * pow(0 - percentage, 2);
    double draws_dev = draws / total * pow(0.5 - percentage, 2);
    double dev = sqrt(wins_dev + loses_dev + draws_dev) / sqrt(total);
    double confidence_p = 0.95;
    double dev_min = percentage + sqrt(2) * dev * erf_inv(-confidence_p);
    double dev_max = percentage + sqrt(2) * dev * erf_inv(confidence_p);
    return (elo_diff(dev_max) - elo_diff(dev_min)) / 2;
}

void start_benchmark(int pipe_main_to_child1[2], int pipe_child1_to_main[2], int pipe_main_to_child2[2], int pipe_child2_to_main[2], pid_t pid1, pid_t pid2)
{

    char answer[256];
    fprintf(OUTPUT_FILE, "Sending 'uci' command to child 1\n");
    communicate_with_child(pipe_main_to_child1, pipe_child1_to_main, "uci\n", answer, 1);
    // fprintf(OUTPUT_FILE, "Answer: %s\n", answer);
    char *token = strtok(answer, "\n");
    if (!handle_uci_command(token))
    {
        fprintf(OUTPUT_FILE, "Error: UCI command to child 1 not handled correctly\n");
        exit(EXIT_FAILURE);
    }

    fprintf(OUTPUT_FILE, "Sending 'uci' command to child 2\n");
    communicate_with_child(pipe_main_to_child2, pipe_child2_to_main, "uci\n", answer, 1);
    token = strtok(answer, "\n");
    if (!handle_uci_command(token))
    {
        fprintf(OUTPUT_FILE, "Error: UCI command to child 2 not handled correctly\n");
        exit(EXIT_FAILURE);
    }

    fprintf(OUTPUT_FILE, "Sending 'isready' command to child 1\n");
    communicate_with_child(pipe_main_to_child1, pipe_child1_to_main, "isready\n", answer, 1);
    if (strcmp(answer, "readyok\n") != 0)
    {
        fprintf(OUTPUT_FILE, "Error: Child 1 not ready\n");
        exit(EXIT_FAILURE);
    }

    fprintf(OUTPUT_FILE, "Sending 'isready' command to child 2\n");
    communicate_with_child(pipe_main_to_child2, pipe_child2_to_main, "isready\n", answer, 1);
    if (strcmp(answer, "readyok\n") != 0)
    {
        fprintf(OUTPUT_FILE, "Error: Child 2 not ready\n");
        exit(EXIT_FAILURE);
    }

    int engine1_time = 100;
    int engine2_time = 100;
    int number_of_rounds = 100;
    int engine1_w_victories = 0;
    int engine1_b_victories = 0;
    int engine2_w_victories = 0;
    int engine2_b_victories = 0;
    int engine1_w_draws = 0;
    int engine2_w_draws = 0;

    // measure the total time taken to play the games
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < number_of_rounds; i++)
    {
        // measure the time taken to play a round
        struct timespec start_round, end_round;
        clock_gettime(CLOCK_MONOTONIC, &start_round);
        int game_result;
        char starting_pos[128];
        fprintf(stdout, "Round %d\n", i);
        fprintf(stdout, "engine1 is white, engine2 is black\n");
        sprintf(starting_pos, "fen \"%s\"", starting_pos_fen[i]);
        fprintf(OUTPUT_FILE, "Sending 'ucinewgame' command to child 1\n");
        communicate_with_child(pipe_main_to_child1, pipe_child1_to_main, "ucinewgame\n", answer, 0);
        fprintf(OUTPUT_FILE, "Sending 'ucinewgame' command to child 2\n");
        communicate_with_child(pipe_main_to_child2, pipe_child2_to_main, "ucinewgame\n", answer, 0);

        game_result = play_game(pipe_main_to_child1, pipe_child1_to_main, pipe_main_to_child2, pipe_child2_to_main, starting_pos, engine1_time, engine2_time);
        if (game_result == 1)
            engine1_w_victories++;
        else if (game_result == -1)
            engine2_b_victories++;
        else
            engine1_w_draws++;
        fprintf(stdout, "Engine 1: %d victories, %d defeats, %d draws\n", engine1_w_victories + engine1_b_victories, engine2_w_victories + engine2_b_victories, engine1_w_draws + engine2_w_draws);

        fprintf(stdout, "engine1 is black, engine2 is white\n");
        fprintf(OUTPUT_FILE, "Sending 'ucinewgame' command to child 1\n");
        communicate_with_child(pipe_main_to_child1, pipe_child1_to_main, "ucinewgame\n", answer, 0);
        fprintf(OUTPUT_FILE, "Sending 'ucinewgame' command to child 2\n");
        communicate_with_child(pipe_main_to_child2, pipe_child2_to_main, "ucinewgame\n", answer, 0);

        game_result = play_game(pipe_main_to_child2, pipe_child2_to_main, pipe_main_to_child1, pipe_child1_to_main, starting_pos, engine2_time, engine1_time);
        if (game_result == 1)
            engine2_w_victories++;
        else if (game_result == -1)
            engine1_b_victories++;
        else
            engine2_w_draws++;
        fprintf(stdout, "Engine 1: %d victories, %d defeats, %d draws\n", engine1_w_victories + engine1_b_victories, engine2_w_victories + engine2_b_victories, engine1_w_draws + engine2_w_draws);
        fprintf(stdout, "Round %d finished\n", i);
        clock_gettime(CLOCK_MONOTONIC, &end_round);
        double elapsed_time_round = (end_round.tv_sec - start_round.tv_sec) + (end_round.tv_nsec - start_round.tv_nsec) / 1e9;
        fprintf(stdout, "Time taken for round %d: %f seconds\n\n", i, elapsed_time_round);
    }

    int draws = engine1_w_draws + engine2_w_draws;
    int engine1_victories = engine1_w_victories + engine1_b_victories;
    int engine2_victories = engine2_w_victories + engine2_b_victories;
    fprintf(stdout, "Engine 1: %d victories (w: %d, b: %d), %d defeats (w: %d, b: %d), %d draws (w: %d, b: %d)\n", engine1_victories, engine1_w_victories, engine1_b_victories, engine2_victories, engine2_b_victories, engine2_w_victories, draws, engine1_w_draws, engine2_w_draws);
    fprintf(stdout, "Engine 2: %d victories (w: %d, b: %d), %d defeats (w: %d, b: %d), %d draws (w: %d, b: %d)\n", engine2_victories, engine2_w_victories, engine2_b_victories, engine1_victories, engine1_b_victories, engine1_w_victories, draws, engine2_w_draws, engine1_w_draws);
    double win_rate1 = (engine1_victories + draws * 0.5) / (engine1_victories + engine2_victories + draws);
    printf("Win rate of engine 1: %f\n", win_rate1);
    double elo_difference = elo_diff(win_rate1);
    double margin_of_error = error_margin(engine1_victories, engine2_victories, draws);
    fprintf(stdout, "Elo difference: %f (margin of error: %f)\n", elo_difference, margin_of_error);

    fprintf(OUTPUT_FILE, "Sending 'quit' command to child 1\n");
    communicate_with_child(pipe_main_to_child1, pipe_child1_to_main, "quit\n", answer, 0);
    fprintf(OUTPUT_FILE, "Sending 'quit' command to child 2\n");
    communicate_with_child(pipe_main_to_child2, pipe_child2_to_main, "quit\n", answer, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    fprintf(stdout, "Total time taken: %f seconds\n", elapsed_time);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
