#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_ARGUMENTS 10
#define MAX_PROCESSES 5
#define LOGSIZE 500000
extern int errno ;
int *childp;
int *count;
bool end = false;
bool alarmB = false;
pid_t chpid = 0;
int logindex = 0;
struct itimerval value, ovalue, pvalue;
char * logarr[LOGSIZE];
int notdone;
int currentPid=0;
int isNumber(char s[])
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
            return 0;
    }
    return 1;
}

typedef struct process
{
    char *name;
    int size;
    char *arguments[MAX_ARGUMENTS + 2];
} process;

void printprocesses(process *p, int numprocesses)
{
    for (int i = 0; i < numprocesses; i++)
    {
        fprintf(stderr, "%s ", p[i].name);
        for (int j = 0; j < p[i].size; j++)
        {
            fprintf(stderr, "%s ", p[i].arguments[j]);
        }
        fprintf(stderr, "\n");
    }
}

// Logs pids and their timestamps of execution
void recordlog(char * s, int pid){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
    sprintf(logarr[logindex],"%s %d %lu\n",s,pid,time_in_micros);
    logindex = logindex + 1;
}

// SIGCHILD handler
void sigh()
{
    pid_t pid;
    int status;
    int index;
    // ovalue=value;
    // int which = ITIMER_REAL;
    // value.it_interval.tv_sec = 0;      
    // value.it_interval.tv_usec = 0;  
    // value.it_value.tv_sec = 0;        
    // value.it_value.tv_usec = 0;     
    // setitimer( which, &value, &ovalue );
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {   
        notdone=notdone-1;
        
        recordlog("Terminate: ", pid );
        for (int i =0; i<*count;i++){
            if (childp[i] == pid){
                childp[i]=-2;
            }
        }
        
        
    }
    
}

// SIGALRM handler
void siga(){
    recordlog("Stop: ", currentPid);
    kill(currentPid,SIGSTOP);
    // ovalue=value;
    // int which = ITIMER_REAL;
    // value.it_interval.tv_sec = 0;   
    // value.it_interval.tv_usec = 0;  
    // value.it_value.tv_sec = 0;           
    // value.it_value.tv_usec = 0;     
    // setitimer( which, &value, &ovalue );

}


int main(int argc, char *argv[])
{
    struct sigaction act;
    sigset_t set;

    memset(&act,0,sizeof act);
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);


    act.sa_mask = set;
    act.sa_handler = sigh;
    act.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &act, NULL);
    signal(SIGALRM, siga);

    childp = mmap(NULL, sizeof(int) * (MAX_PROCESSES + 1), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int i = 0; i<LOGSIZE;i++){
        logarr[i]=(char *)malloc(sizeof(char)*100);
    }
    process *processes = (process *)malloc(sizeof(process));

    // usage error checking
    if (argc < 3)
    {
        fprintf(stderr, "Usage: schedule [milliseconds (positive integer)] [prog1 [prog1 args...]] [prog2 [prog2 args...]] ...\n");
        free(processes);
        munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
        munmap(count, sizeof(int));
        return 0;
    }
    if (!isNumber(argv[1]))
    {
        fprintf(stderr, "Usage: schedule [milliseconds (positive integer)] [prog1 [prog1 args...]] [prog2 [prog2 args...]] ...\n");
        free(processes);
        munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
        munmap(count, sizeof(int));
        return 0;
    }
    int quantum = atoi(argv[1]);
    int index = 2;
    int numprocesses = 0;
    
    // parse and add all command line arguments to theprocess array
    while (index < argc)
    {
        if(numprocesses>MAX_PROCESSES){
            fprintf(stderr, "ERROR: number of processes given exceeds max process size of %d",MAX_PROCESSES);
            munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
            munmap(count, sizeof(int));
            for (int i = 0; i < numprocesses; i++)
            {
                free(processes[i].name);
                for (int j = 0; j < processes[i].size; j++)
                {
                    free(processes[i].arguments[j]);
                }
            }
            free(processes);
            return 0;
        }
        process p;

        int size = 0;
        p.name = (char *)malloc(sizeof(char) * (strlen(argv[index]) + 1));
        strcpy(p.name, argv[index]);
        numprocesses++;
        
        index++;
        // delimiter for separating each process and their arguments
        p.arguments[size] = (char *)malloc(sizeof(char) * (strlen(p.name) + 1));
        strcpy(p.arguments[size], p.name);
        size++;
        while (index < argc && strcmp(argv[index], ":") != 0)
        {
            p.arguments[size] = (char *)malloc(sizeof(char) * (strlen(argv[index]) + 1));
            strcpy(p.arguments[size], argv[index]);
            size++;
            index++;
        }

        p.size = size;
        p.arguments[size] = NULL;
        size = 0;
        // extend process array size upon appending process
        processes[numprocesses - 1] = p;

        process *reallocproccess = realloc(processes, (numprocesses + 1) * sizeof(process));
        if (reallocproccess)
        {
            processes = reallocproccess;
        }
        else
        {
            fprintf(stderr, "ERROR: allocating memory for process details");
            munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
            munmap(count, sizeof(int));
            for (int i = 0; i < numprocesses; i++)
            {
                free(processes[i].name);
                for (int j = 0; j < processes[i].size; j++)
                {
                    free(processes[i].arguments[j]);
                }
            }
            free(processes);
            return 0;
        }
        if (index < argc && strcmp(argv[index], ":") == 0)
        {
            index++;
        }
    }

    *count = 0;
    for (int i = 0; i < numprocesses; i++)
    {
        int pidC = fork();
        if (pidC == -1)
        {
            fprintf(stderr, "ERROR: failed to schedule processes");
            munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
            munmap(count, sizeof(int));
            for (int i = 0; i < numprocesses; i++)
            {
                free(processes[i].name);
                for (int j = 0; j < processes[i].size; j++)
                {
                    free(processes[i].arguments[j]);
                }
            }
            free(processes);
            return 0;
            for (int i = 0; i < *count; i++)
            {
                kill(childp[i], SIGKILL);
            }
        }
        if (pidC == 0)
        {
            int mypid =getpid();
            
            childp[i]=mypid;
            *count = *count + 1;
            // fprintf(stderr, "start: %d %s\n", mypid, processes[i].arguments[1]);
            kill(mypid, SIGSTOP);
            // fprintf(stderr, "SIGCONT: %d\n", mypid);
            execv(processes[i].name, processes[i].arguments);
            exit(-1);
        }
    }
    while (*count!=numprocesses)
        ;
    
    for (int i = 0; i < *count; i++)
    {
        fprintf(stderr, "child %d: %d\n", i, childp[i]);
    }
    bool flag = false;
    int errnum;
    notdone = *count;

    while (notdone)
    {
        
        for (int i = 0; i < *count; i++)
        {
            if (childp[i]!=-2){
                currentPid=childp[i];
                kill(childp[i], SIGCONT);
                recordlog("CONT: ", childp[i]);
                int which = ITIMER_REAL;
                getitimer( which, &pvalue );
                value.it_interval.tv_sec = 0;
                value.it_interval.tv_usec = 0;
                value.it_value.tv_sec = 0; 
                value.it_value.tv_usec = quantum*1000;  

                int result = setitimer( which, &value, &ovalue );
                // recordlog("Timer: ", childp[i] );
                // recordlog("Timer Result: ", result );
                sigset_t myset;
                sigemptyset(&myset);
                sigsuspend(&myset);

                struct itimerval remaining;
                getitimer(which,&remaining);
                recordlog("Timer Remaining: ", 1000000 * remaining.it_value.tv_sec + remaining.it_value.tv_usec );
                
                }
            }
        }
    int status;
    int wpid;
    while ((wpid = wait(&status)) > 0);
    munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
    munmap(count, sizeof(int));
    FILE *fp;
    fp = fopen("log.txt", "w");
    for (int i = 0; i < logindex; i++)
    {
        
        fprintf(fp, "%s", logarr[i]);

    }
     fclose(fp);
    for (int i = 0; i < numprocesses; i++)
    {
        free(processes[i].name);
        for (int j = 0; j < processes[i].size; j++)
        {
            free(processes[i].arguments[j]);
        }
    }
    free(processes);
    fprintf(stderr, "Log Index: %d\n", logindex);
}
