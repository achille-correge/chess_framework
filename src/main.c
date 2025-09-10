#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "communication.h"
#include "games.h"
#include "parameters.h"

int main()
{

    // Redirect stderr to /dev/null to hide debug messages

    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null != -1)
    {
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }

    int pipe_main_to_child1[2];
    int pipe_child1_to_main[2];
    int pipe_main_to_child2[2];
    int pipe_child2_to_main[2];

    create_pipes(pipe_main_to_child1, pipe_child1_to_main);
    create_pipes(pipe_main_to_child2, pipe_child2_to_main);

    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0)
    {
        setup_child_process(pipe_main_to_child1, pipe_child1_to_main, ENGINES_DIR ENGINE_1_NAME);
    }
    else
    {
        pid_t pid2 = fork();
        if (pid2 == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0)
        {
            setup_child_process(pipe_main_to_child2, pipe_child2_to_main, ENGINES_DIR ENGINE_2_NAME);
        }
        else
        {

            close(pipe_main_to_child1[0]); // Close read end of the first pipe for each child
            close(pipe_main_to_child2[0]);
            close(pipe_child1_to_main[1]); // Close write end of the second pipe for each child
            close(pipe_child2_to_main[1]);

            start_benchmark(pipe_main_to_child1, pipe_child1_to_main, pipe_main_to_child2, pipe_child2_to_main, pid1, pid2);
        }
    }

    return 0;
}