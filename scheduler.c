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

#define MAX_ARGUMENTS 10
#define MAX_PROCESSES 101

int *childp;
int *count;
bool end = false;
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
    char *arguments[MAX_ARGUMENTS + 1];
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
void sigh()
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        chpid = pid;
    }
}
int main(int argc, char *argv[])
{

    // array for keeping track of processes
    signal(SIGCHLD, sigh);
    childp = mmap(NULL, sizeof(int) * (MAX_PROCESSES + 1), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

    process *processes = (process *)malloc(sizeof(process));

    // usage error checking
    if (argc < 3)
    {
        fprintf(stderr, "Usage: schedule [milliseconds] [prog1 [prog1 args...]] [prog2 [prog2 args...]] ...\n");
        return 0;
    }
    if (!isNumber(argv[1]))
    {
        fprintf(stderr, "Second argument should be a numerical value representing milliseconds\n");
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
            return 0;
            for (int i = 0; i < *count; i++)
            {
                kill(childp[i], SIGKILL);
            }
        }
        if (pidC == 0)
        {
            *count = *count + 1;
            int mypid = getpid();

            int index = -1;
            for (int j = 0; j < MAX_PROCESSES; j++)
            {
                if (childp[j] == 0)
                {
                    index = j;
                    break;
                }
            }
            if (index == -1)
                return 0;

            childp[index] = getpid();
            kill(mypid, SIGSTOP);
            char *name = (char *)malloc(sizeof(char) * (strlen(processes[i].name) + 3));
            name[0] = '.';
            name[1] = '/';
            strcat(name, processes[i].name);
            execvp(name, processes[i].arguments);
            exit(-1);
        }
    }
    while (*count == 0)
        ;
    while (childp[0] == 0)
        ;

    struct timeval stop, start;
    for (int i = 0; i < *count; i++)
    {
        fprintf(stderr, "child %d: %d\n", i, childp[i]);
    }
    while (*count)
    {
        for (int i = 0; i < *count; i++)
        {

            gettimeofday(&start, NULL);
            kill(childp[i], SIGCONT);
            gettimeofday(&stop, NULL);
            int time_spent = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000;

            // timer for quantum

            while (time_spent < quantum)
            {
                // if end flag is activated by child, then we break the timer
                // and move on to remove it from array of pending processes
                if (chpid == childp[i])
                {
                    break;
                }
                gettimeofday(&stop, NULL);
                time_spent = ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec) / 1000;
            }
            // if scheduled program has not completed then we pause it
            if (chpid != childp[i])
            {
                kill(childp[i], SIGSTOP);
            }
            else
            {
                // if scheduled program completes, remove it from list of children
                for (int j = i; j < *count - 1; j++)
                {
                    childp[j] = childp[j + 1]; // assign arr[i+1] to arr[i]
                }
                *count = *count - 1;

                i--;
            }
        }
    }
    wait(0);
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
