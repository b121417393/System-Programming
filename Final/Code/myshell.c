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
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

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

char* argVect[256];
char* temp[256];

sigjmp_buf jumpBuf;
volatile sig_atomic_t hasChild = 0;
pid_t childPid;
const long long nspersec = 1000000000;

long long timespec2sec(struct timespec ts) {
    long long ns = ts.tv_nsec;
    ns += ts.tv_sec * nspersec;
    return ns;
}

double timeval2sec(struct timeval input) {
    long long us = input.tv_sec*1000000;
    us += input.tv_usec;
    return (double)us/1000000.0;
}


void ctrC_handler(int sigNumber) {
    if (hasChild) {
        kill(childPid, sigNumber);
        hasChild = 0;
    } 
    else 
    {
        if (argVect[0] == NULL) {
            ungetc('\n', stdin);ungetc('c', stdin);ungetc('^', stdin);
            siglongjmp(jumpBuf, 1);
        } 
        else {
            fprintf(stderr, "info, 處理字串時使用者按下ctr-c\n");
        }
    }
}

void quit_handler(int sigNumber) {
    if (hasChild) {
        kill(childPid, sigNumber);
        hasChild = 0;
    }
    else
    {
		if (argVect[0] == NULL) {
			ungetc('\n', stdin);ungetc('\\', stdin);ungetc('^', stdin);
            siglongjmp(jumpBuf, 1);
        } 
        else {
            fprintf(stderr, "info, 處理字串時使用者按下ctr-\\\n");
        }
    }
}


void parseString(char* str, char** cmd) {
    int idx=0;
    char* retPtr;
    //printf("%s\n", str);
    retPtr=strtok(str, " \n");
    while(retPtr != NULL) {
        //printf("token =%s\n", retPtr);
        //if(strlen(retPtr)==0) continue;
        argVect[idx++] = retPtr;
        if (idx==1)
            *cmd = retPtr;
        retPtr=strtok(NULL, " \n");
    }
    argVect[idx]=NULL;
}

int main (int argc, char** argv) {
    char cmdLine[4096];
    char hostName[256];
    char cwd[256];
    char* exeName;
    int pid, pos, wstatus;
    struct rusage resUsage; 
    struct timespec statTime, endTime;

    signal(SIGINT, ctrC_handler);
    signal(SIGQUIT, quit_handler);
    signal(SIGTSTP, SIG_IGN);

    while(1) {
        char* showPath;
        char* loginName;
        int homeLen = 0;
        hasChild = 0;
        argVect[0]=NULL;

        loginName = getlogin();
        gethostname(hostName, 256);

        getcwd(cwd, 256);
        pos=strspn(getenv("HOME"), cwd);
        homeLen = strlen(getenv("HOME"));

        if(pos>=homeLen) {
            cwd[pos-1]='~';
            showPath=&cwd[pos-1];
        }
        else
            showPath=cwd;

        printf(LIGHT_GREEN"%s@%s:", loginName, hostName);
        printf(BLU_BOLD"%s>> " NONE, showPath);

        sigsetjmp(jumpBuf, 1);
        fgets(cmdLine, 4096, stdin);

        if (strlen(cmdLine)>1) 
            parseString(cmdLine, &exeName);
        else
            continue;
        if (strcmp(exeName, "^c") == 0) { 
            printf("\n");
            continue;
        }
        if (strcmp(exeName, "^\\") == 0) { 
            printf("\n");
            continue;
        }
        if (strcmp(exeName, "exit") == 0)
            break;
        if (strcmp(exeName, "cd") == 0) {
            if (strcmp(argVect[1], "~")==0)
                chdir(getenv("HOME"));
            else
                chdir(argVect[1]);
            continue;
        }
        clock_gettime(CLOCK_MONOTONIC, &statTime);
        pid = fork();
        
        if (pid == 0) {	
			temp[0]=argVect[0];
			temp[1]=NULL;
			int fd;
			char path[64];
			
			if(argVect[1]==NULL)
				execvp(exeName, argVect);
				
			else if(strcmp(argVect[1],">")!=0 && strcmp(argVect[1],"<")!=0)
				execvp(exeName, argVect);
				
			else if(strcmp(argVect[1],">")==0)
			{
				close(1);
				strcat(path,"./");
				strcat(path,argVect[2]);
				fd = open(path,O_WRONLY);
				execvp(exeName, temp);
			}
			else if(strcmp(argVect[1],"<")==0)
			{
				close(0);
				strcat(path,"./");
				strcat(path,argVect[2]);
				fd = open(path,O_RDONLY);
				execvp(exeName, temp);
			}
			close(fd);
        } 
        
        else {
            childPid = pid;
            hasChild = 1;
            wait3(&wstatus, 0, &resUsage);
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            double sysTime = timeval2sec(resUsage.ru_stime);
            double usrTime = timeval2sec(resUsage.ru_utime);
            printf("\n");
            long long elapse = timespec2sec(endTime)-timespec2sec(statTime);
            printf(RED"經過時間:                       "YELLOW"%lld.%llds\n",elapse/nspersec, elapse%nspersec);
            printf(RED"CPU花在執行程式的時間:	         "YELLOW"%fs\n"
                   RED"CPU於usr mode執行此程式所花的時間："YELLOW"%fs\n"
                   RED"CPU於krl mode執行此程式所花的時間："YELLOW"%fs\n", sysTime+usrTime , usrTime, sysTime);
            printf(RED"Page fault，但沒有造成I/O：      "YELLOW"%ld\n", resUsage.ru_minflt);
            printf(RED"Page fault，並且觸發I/O:        "YELLOW"%ld\n", resUsage.ru_majflt);
            printf(RED"自願性的context switch：        "YELLOW"%ld\n", resUsage.ru_nvcsw);
            printf(RED"非自願性的context switch:       "YELLOW"%ld\n", resUsage.ru_nivcsw);
            printf(RED "return value of " YELLOW "%s" RED " is " YELLOW "%d\n", 
                exeName, WEXITSTATUS(wstatus));
            if (WIFSIGNALED(wstatus))
                printf(RED"the child process was terminated by a signal "YELLOW"%d"RED
                    ", named " YELLOW "%s.\n",  WTERMSIG(wstatus), sys_siglist[WTERMSIG(wstatus)]);
            printf(NONE);
        }
    }
}
