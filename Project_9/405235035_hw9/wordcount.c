#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int pipefd[2];
    int wstat, pid1, pid2;
    pipe(pipefd); //Create a pipeline

    pid1 = fork(); //Generate the first child to execute cat "filename".
    if (pid1 == 0)
    {
        close(1);
        dup(pipefd[1]); //Connect pipe to the original stdout
        close(pipefd[1]);
        close(pipefd[0]);

        //Declares an array of pointers to be used as execv's parameters , and initialize its value.
        char *parameter[3] = {"cat", argv[1], NULL};
        execv("/bin/cat", parameter);
    }

    if (pid1 > 0) //Generate second child to execute wc stdin.
    {
        pid2 = fork();
        if (pid2 == 0)
        {
            close(0);
            dup(pipefd[0]); //Connect pipe to the original stdin
            close(pipefd[1]);
            close(pipefd[0]);

            //Declares an array of pointers to be used as execv's parameters , and initialize its value.
            char *parameter[2] = {"wc", NULL};
            printf("newlines  words  byte counts\n");
            execv("/usr/bin/wc", parameter);
        }
    }

    //Unused pipes must be closed.
    close(pipefd[0]);
    close(pipefd[1]);
    wait(&wstat);
    wait(&wstat);
}
