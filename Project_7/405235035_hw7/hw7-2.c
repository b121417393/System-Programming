#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <linux/limits.h>

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
//Store path strings that are split from environment variables.
char *envpath[256];

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

//Split the environment variable into several individual paths.
void parsepath(char *path, char **envpath)
{
    int idx = 0;
    char *retPtr;
    retPtr = strtok(path, ":");
    while (retPtr != NULL)
    {
        envpath[idx++] = retPtr;
        retPtr = strtok(NULL, ":");
    }
    envpath[idx] = NULL;
}

int main(int argc, char *argv[], char *envp[])
{
    char cmdLine[4096];
    char hostName[256];
    char cwd[256];
    char *exeName;
    int pid, pos, wstatus;

    //Get the environment variable "PATH".
    char *path = getenv("PATH");
    //Split the environment variable "PATH".
    parsepath(path, envpath);

    while (1)
    {
        char *showPath;
        char *loginName = getlogin();
        int homeLen = 0;
        gethostname(hostName, 256);

        getcwd(cwd, 256);
        pos = strspn(getenv("HOME"), cwd);
        homeLen = strlen(getenv("HOME"));

        if (pos >= homeLen)
        {
            cwd[pos - 1] = '~';
            showPath = &cwd[pos - 1];
        }
        else
            showPath = cwd;

        printf(LIGHT_GREEN "%s@%s:", loginName, hostName);
        printf(BLU_BOLD "%s>> ", showPath);
        printf(NONE);

        //Initialize argVect.
        for (int i = 0; i < 256; i++)
            argVect[i] = NULL;

        fgets(cmdLine, 4096, stdin);
        if (strlen(cmdLine) > 1)
            parseString(cmdLine, &exeName);
        else
            continue;
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

        //Find the location of the execution file from the path
        int i = 0;
        int flag = 0;
        while (envpath[i] != NULL)
        {
            DIR *dir;
            struct dirent *ent;
            dir = opendir(envpath[i]);
            if (dir == NULL)
            {
                i++;
                continue;
            }
            ent = readdir(dir);

            while (ent != NULL)
            {
                if (strcmp(argVect[0], ent->d_name) == 0)
                {
                    flag = 1;
                    break;
                }
                else
                    ent = readdir(dir);
            }

            closedir(dir);
            if (flag == 1)
                break;
            i++;
        }

        char temp[32];
        char temp_ab[32];
        //If the execution file is in the current directory
        if (argVect[0][0] == '.' && argVect[0][1] == '/')
        {
            strcpy(temp_ab, argVect[0]);
            int len = strlen(argVect[0]);
            for (int i = 0; i < len - 2; i++)
                temp[i] = argVect[0][i + 2];
            temp[i] = '\0';
            flag = 2;
            strcmp(argVect[0], temp);
        }

        //If exeution file can not found.
        if (flag == 0)
        {
            printf("%s:command not found\n", argVect[0]);
            continue;
        }

        //The absolute path to the execution file.
        char absolute_path[256];
        //If the execution file is in the "PATH"
        if (flag == 1)
        {
            //Synthesizing paths as absolute paths
            strcpy(absolute_path, envpath[i]);
            strcat(absolute_path, "/");
            strcat(absolute_path, argVect[0]);
        }
        //If the execution file is in the current directory
        else if (flag == 2)
            strcpy(absolute_path, temp_ab);

        pid = fork();
        if (pid == 0)
        {
            if (execle(absolute_path, argVect[0], argVect[1], argVect[2], argVect[3], argVect[4], argVect[5], argVect[6], NULL, envp) == -1)
            {
                perror("myShell");
                exit(errno * -1);
            }
        }
        else
        {
            wait(&wstatus);
            printf(RED "return value of " YELLOW "%s" RED " is " YELLOW "%d\n", exeName, WEXITSTATUS(wstatus));
            printf(NONE);
        }
    }
}
