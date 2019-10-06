#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <setjmp.h>
#include <sys/resource.h>

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
#define RED_BOLD "\x1b[;31;1m"
#define BLU_BOLD "\x1b[;34;1m"
#define YEL_BOLD "\x1b[;33;1m"
#define GRN_BOLD "\x1b[;32;1m"
#define CYAN_BOLD_ITALIC "\x1b[;36;1;3m"
#define RESET "\x1b[0;m"

char *argVect[256];

sigjmp_buf jumpBuf, jumpBuf2;
volatile sig_atomic_t hasChild = 0;
pid_t childPid;
const long long nspersec = 1000000000;

long long timespec2sec(struct timespec ts)
{
    long long ns = ts.tv_nsec;
    ns += ts.tv_sec * nspersec;
    return ns;
}

double timeval2sec(struct timeval input)
{
    long long us = input.tv_sec * 1000000;
    us += input.tv_usec;
    return (double)us / 1000000.0;
}

void ctrC_handler(int sigNumber)
{
    if (hasChild)
    {
        kill(childPid, sigNumber);
        hasChild = 0;
        argVect[0] == NULL;
    }
    else
    {
        if (argVect[0] == NULL)
        {
            ungetc('\n', stdin);
            ungetc('c', stdin);
            ungetc('^', stdin);
            siglongjmp(jumpBuf, 1);
        }
        else
            fprintf(stderr, "info, ??????å­?ä¸²æ??ä½¿ç?¨è?????ä¸?ctr-c\n");
    }
}

void child_handler(int sigNumber)
{
    childPid = -1;
    hasChild = 0;
}

void default_handler(int sigNumber)
{
    printf(RED "signal # is %d\n", sigNumber);
    siglongjmp(jumpBuf2, 1);
}

void parseString(char *str, char **cmd)
{
    int idx = 0;
    char *retPtr;
    retPtr = strtok(str, " \n");
    while (retPtr != NULL)
    {
        argVect[idx++] = retPtr;
        if (idx == 1)
            *cmd = retPtr;
        retPtr = strtok(NULL, " \n");
    }
    argVect[idx] = NULL;
}

int main(int argc, char **argv)
{
    char cmdLine[4096];
    char hostName[256];
    char cwd[256];
    char *exeName;
    int pid, pos, wstatus;
    
    struct rusage resUsage;
    struct timespec statTime, endTime;

    //Set the sigaction's related parameters
    struct sigaction ctrC_act, child_act, default_act;

    //Set default signal processing
    default_act.sa_handler = default_handler;
    for (int i = 0; i < 31; i++)
    {
        sigaddset(&default_act.sa_mask, i);
        sigaction(i, &default_act, NULL);
    }

    //Set ctrl+C signal processing
    ctrC_act.sa_handler = ctrC_handler;
    sigaddset(&ctrC_act.sa_mask, SIGINT);
    ctrC_act.sa_flags = SA_RESTART;
    sigaction(SIGINT, &ctrC_act, NULL);

    //Set Child signal processing
    child_act.sa_handler = child_handler;
    sigaddset(&child_act.sa_mask, SIGCHLD);
    child_act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &child_act, NULL);

    //Default signal jump point.
    sigsetjmp(jumpBuf2, 1);
    while (1)
    {
        hasChild = 0;
        argVect[0] = NULL;

        //Print hints character
        char *showPath;
        char *loginName;
        int homeLen = 0;
        loginName = getlogin();
        gethostname(hostName, 256);
        getcwd(cwd, 256);
        pos = strspn(getenv("HOME"), cwd);
        homeLen = strlen(getenv("HOME"));
        if (pos >= homeLen)
            cwd[pos - 1] = '~', showPath = &cwd[pos - 1];
        else
            showPath = cwd;
        printf(LIGHT_GREEN "%s@%s:", loginName, hostName);
        printf(BLU_BOLD "%s>> " NONE, showPath);

        //Ctrl+C signal jump point
        sigsetjmp(jumpBuf, 1);
        fgets(cmdLine, 4096, stdin);

        if (strlen(cmdLine) > 1)
            parseString(cmdLine, &exeName);
        else
            continue;

        if (strcmp(exeName, "^c") == 0)
        {
            printf("\n");
            continue;
        }

        if (strcmp(exeName, "exit") == 0)
            break;

        if (strcmp(exeName, "cd") == 0)
        {
            if (strcmp(argVect[1], "~") == 0)
                chdir(getenv("HOME"));
            else
                chdir(argVect[1]);
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &statTime);
        pid = fork();
        if (pid == 0)
            execvp(exeName, argVect);
        else
        {
            childPid = pid;
            hasChild = 1;
            wait3(&wstatus, 0, &resUsage);

            clock_gettime(CLOCK_MONOTONIC, &endTime);

            double sysTime = timeval2sec(resUsage.ru_stime);
            double usrTime = timeval2sec(resUsage.ru_utime);

            long long elapse = timespec2sec(endTime) - timespec2sec(statTime);
            printf(RED "real:" YELLOW "%lld.%lld sec\n", elapse / nspersec, elapse % nspersec);
            printf(RED "user:" YELLOW "%f sec\n" RED "sys :" YELLOW "%f sec\n", usrTime, sysTime);
            printf(RED "return value of " YELLOW "%s" RED " is " YELLOW "%d\n",
                   exeName, WEXITSTATUS(wstatus));

            if (WIFSIGNALED(wstatus))
                printf(RED "the child process was terminated by a signal " YELLOW "%d" RED
                           ", named " YELLOW "%s.\n",
                       WTERMSIG(wstatus), sys_siglist[WTERMSIG(wstatus)]);
            printf(NONE);
        }
    }
}
