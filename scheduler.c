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
#define MAX_PROCESSES 101
extern int errno ;
int *childp;
int *count;
bool end = false;
bool alarmB = false;
pid_t chpid = 0;
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
bool checkChildArr(){
    for (int i = 0; i < *count; i++)
    {
        if(childp[i]==0){
            return false;
        }
    }
}
void sigh()
{
    pid_t pid;
    int status;
    int index;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // fprintf(stderr, "terminated: %d ", pid);
        for (int i =0; i<*count;i++){
            if (childp[i] == pid){
                index = i;
            }
        }
        for (int j = index; j < *count; j++)
                {
                    childp[j] = childp[j + 1];
                }
                *count = *count - 1;
    }
    end = true;
}
void siga(){
    fprintf(stderr, "alarm: %d\n",alarmB);
    end=true;
    alarmB=true;
}
int main(int argc, char *argv[])
{

    // array for keeping track of processes
    signal(SIGCHLD, sigh);
    signal(SIGALRM, siga);
    childp = mmap(NULL, sizeof(int) * (MAX_PROCESSES + 1), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

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
    // parse and add all command line arguments to process array
    while (index < argc)
    {

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
            fprintf(stderr, "SIGCONT: %d\n", mypid);
            execv(processes[i].name, processes[i].arguments);
            exit(-1);
        }
    }
    while (*count!=numprocesses)
        ;
    
    // for (int i = 0; i < *count; i++)
    // {
    //     fprintf(stderr, "child %d: %d\n", i, childp[i]);
    // }
    // fprintf(stderr, "count: %d\n",*count);
    bool flag = false;
    int errnum;
    while (*count)
    {
        
        for (int i = 0; i < *count; i++)
        {
            int cpid=childp[i];
            kill(childp[i], SIGCONT);
            // fprintf(stderr, "cont: %d\n", childp[i]);
            int status = usleep(quantum*1000);
            // fprintf(stderr, "status: %d\n", status);
            if(status==0 && !end){
                kill(childp[i], SIGSTOP);
                // fprintf(stderr, "stop: %d\n", childp[i]);
            }
            else{
                // fprintf(stderr, "end: %d\n", childp[i]);
                i--;
                end=false;
            }
        


        }


    }
    int status;
    int wpid;
    while ((wpid = wait(&status)) > 0);
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
}
