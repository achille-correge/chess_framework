#ifndef COMMUNICATION_H
#define COMMUNICATION_H

void create_pipes(int pipe_main_to_child[2], int pipe_child_to_main[2]);
void setup_child_process(int pipe_main_to_child[2], int pipe_child_to_main[2], const char *program_name);
char *communicate_with_child(int pipe_main_to_child[2], int pipe_child_to_main[2], const char *message, char answer[256], int expects_answer);

#endif