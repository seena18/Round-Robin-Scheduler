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

//GLOBAL VARIABLES
#define MAX_ARGUMENTS 10
#define MAX_PROCESSES 101

extern int errno ;
int *childp;
int *count;
int notdone;
int currentPid=0;

//function to check  if a string is a number
int isNumber(char s[])
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
            return 0;
    }
    return 1;
}

//struct to hold process information as we parse
typedef struct process
{
    char *name;
    int size;
    char *arguments[MAX_ARGUMENTS + 2];
} process;

// SIGALRM handler
void siga(){
    kill(currentPid,SIGSTOP);
}


int main(int argc, char *argv[])
{
    //setup SIGALRM signal handler
    signal(SIGALRM, siga);

    //chidp structure holds child PIDs
    //count holds number of forked children
    childp = mmap(NULL, sizeof(int) * (MAX_PROCESSES + 1), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    //array to hold parsed process info
    process *processes = (process *)malloc(sizeof(process));

    //setitimer declared values
    struct itimerval value, ovalue, pvalue;

    // usage error checking

    //if not enough arguments
    if (argc < 3)
    {
        fprintf(stderr, "Usage: schedule [milliseconds (positive integer)] [prog1 [prog1 args...]] [prog2 [prog2 args...]] ...\n");
        free(processes);
        munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
        munmap(count, sizeof(int));
        return 0;
    }
    //if quantum is not numerical
    if (!isNumber(argv[1]))
    {
        fprintf(stderr, "Usage: schedule [milliseconds (positive integer)] [prog1 [prog1 args...]] [prog2 [prog2 args...]] ...\n");
        free(processes);
        munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
        munmap(count, sizeof(int));
        return 0;
    }

    //setting quantum and initializing parser variables
    int quantum = atoi(argv[1]);
    int index = 2;
    int numprocesses = 0;
    
    // parse and add all command line arguments to theprocess array
    while (index < argc)
    {
        numprocesses++;
        //error handling if too many processes given
        if(numprocesses>MAX_PROCESSES){
            fprintf(stderr, "ERROR: number of processes given exceeds max process size of %d",MAX_PROCESSES);
            munmap(childp, sizeof(int) * (MAX_PROCESSES + 1));
            munmap(count, sizeof(int));
            for (int i = 0; i < numprocesses-1; i++)
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

        //error handling if array expansion fails

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
    //initialize process counter
    *count = 0;

    //iterate over the process array and fork children/setup for exec
    for (int i = 0; i < numprocesses; i++)
    {
        int pidC = fork();
        //if fork fails handle error
        if (pidC == -1)
        {
            fprintf(stderr, "ERROR: failed to schedule processes\n");
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
        //if child: setup for exec, record information in child array,increment child fork count
        if (pidC == 0)
        {
            int mypid =getpid();
            childp[i]=mypid;
            *count = *count + 1;
            //child stops itself after record initial information
            kill(mypid, SIGSTOP);
            execv(processes[i].name, processes[i].arguments);
            exit(-1);
        }
    }
    //parent waits for child count to equal the number of parsed processes
    while (*count!=numprocesses)
        ;
    
    int errnum;
    notdone = *count;

    //initialze wait() variables
    int waitValue,stat;

    //notdone is initialized to number of forked processes
    //while notdone is not 0 we iterate over the child pids
    while (notdone)
    {
        //iterate inorder 
        for (int i = 0; i < *count; i++)
        {
            //-2 flag indicates the child has terminated
            if (childp[i]!=-2){
                //store current childs pid globally and send continue signal
                currentPid=childp[i];
                kill(childp[i], SIGCONT);

                //initialize timer to quantum value
                int which = ITIMER_REAL;
                getitimer( which, &pvalue );
                value.it_interval.tv_sec = 0;
                value.it_interval.tv_usec = 0;
                value.it_value.tv_sec = quantum / 1000; 
                value.it_value.tv_usec = (quantum % 1000)*1000;
                if (setitimer(ITIMER_REAL, &value, NULL) == -1) {
                    perror("Error setting timer");
                }

                //wait for child status from SIGALRM or child termination
                do
                {
                    waitValue = waitpid(currentPid, &stat, WUNTRACED);
                } while (waitValue == -1 && errno == EINTR);

                //if child terminated set flag and decrement notdone counter
                if (WIFEXITED(stat))
                {
                    childp[i]=-2;
                    notdone=notdone-1;
                }
                
            }
        }
    }

    //free memory
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
