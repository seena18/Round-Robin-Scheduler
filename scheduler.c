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
#define logsize 10000
extern int errno ;
int *childp;
int *count;
bool end = false;
bool alarmB = false;
pid_t chpid = 0;
int *logindex;

char * logarr[logsize];
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

//SIGCHILD handler
void sigh()
{
    pid_t pid;
    int status;
    int index;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
        sprintf(logarr[*logindex], "terminated %d %lu\n", pid,time_in_micros);
        *logindex=*logindex+1;
        
        for (int i =0; i<*count;i++){
            if (childp[i] == pid){
                childp[i]==-1;
            }
        }
        for (int j = index; j < *count; j++)
                {
                    childp[j] = childp[j + 1];
                }
                *count = *count - 1;
    }
    
}
void siga(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGALRM: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigu1(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGUSR1: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigu2(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGUSR2: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sighup(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGHUP: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigf(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGFPE: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigi(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGINT: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigt(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGTERM: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigabrt(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGABRT: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigbus(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGBUS: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigill(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGILL: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigprof(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGILL: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigttou(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGTTOU: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigttin(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGTTIN: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigemt(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGEMT: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigpwr(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGPWR: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigio(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGIO: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigiot(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGIOT: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigpipe(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGPIPE: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigpoll(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGPOLL: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigsegv(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGSEGV: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigsys(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGSYS: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigtrap(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGTRAP: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigurg(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGURM: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigvalrm(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGVALRM: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigxcpu(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGXCPU: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigxfsz(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "SIGXFSZ: %lu",time_in_micros);
        *logindex=*logindex+1;
}
void sigwinch(){
    struct timeval tv;
        gettimeofday(&tv,NULL);
        unsigned long  time_in_micros = 1000000 * tv.tv_sec + abs(tv.tv_usec);
        sprintf(logarr[*logindex], "sigwinch: %lu",time_in_micros);
        *logindex=*logindex+1;
}

int main(int argc, char *argv[])
{
    // array for keeping track of processes
    signal(SIGCHLD, sigh);
    signal(SIGALRM, siga);
    signal(SIGUSR1, sigu1);
    signal(SIGUSR2, sigu2);
    signal(SIGHUP, sighup);
    signal(SIGFPE, sigf);
    signal(SIGINT, sigf);
    signal(SIGTERM, sigt);
    signal(SIGABRT, sigabrt);
    signal(SIGBUS, sigbus);
    signal(SIGILL, sigill);
    signal(SIGPROF, sigprof);
    signal(SIGTTOU, sigttou);
    signal(SIGTTIN, sigttou);
    signal(SIGPWR, sigpwr);
    signal(SIGIO, sigio);
    signal(SIGIOT, sigiot);
    signal(SIGPIPE, sigpipe);
    signal(SIGPOLL, sigpoll);
    signal(SIGSEGV, sigsegv);
    signal(SIGSYS, sigsys);
    signal(SIGTRAP, sigtrap);
    signal(SIGURG, sigurg);
    signal(SIGVTALRM, sigvalrm);
    signal(SIGXCPU, sigxcpu);
    signal(SIGXFSZ, sigxfsz);
    signal(SIGWINCH, sigwinch);
    childp = mmap(NULL, sizeof(int) * (MAX_PROCESSES + 1), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    logindex=mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int i = 0; i<logsize;i++){
        logarr[i]=mmap(NULL, sizeof(char)*100, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
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
    // fprintf(stderr, "count: %d\n",*count);
    bool flag = false;
    int errnum;
    while (*count)
    {
        
        for (int i = 0; i < *count; i++)
        {
            int cpid=childp[i];
            kill(childp[i], SIGCONT);
            struct timeval tv;
            gettimeofday(&tv,NULL);
            unsigned long  time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
            sprintf(logarr[*logindex], "cont %d: %lu ", childp[i],time_in_micros);
            *logindex=*logindex+1;
            // fprintf(stderr, "cont: %d\n", childp[i]);
            struct timespec a;
            a.tv_sec=quantum/1000;
            a.tv_nsec=(quantum%1000)*1000000;
            struct timespec b;
            b.tv_sec=0;
            b.tv_nsec=0;
            int status = nanosleep(&a,&b);
            if(status==-1){
                errnum = errno;
                gettimeofday(&tv,NULL);
                time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
                sprintf(logarr[*logindex], "timer status: %s  %lu %lu %lu", strerror( errnum ), time_in_micros,b.tv_sec,b.tv_nsec);
            }
            


            

            *logindex=*logindex+1;
            if(status==0){
                kill(childp[i], SIGSTOP);
                gettimeofday(&tv,NULL);
                time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
                sprintf(logarr[*logindex], "stop %d: %lu\n", childp[i], time_in_micros);
                *logindex=*logindex+1;
                
                // fprintf(stderr, "stop: %d\n", childp[i]);
            }
            else if (end || status==-1){
                
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
    fprintf(stderr,"logindex: %d\n",*logindex);
    for (int i = 0; i < 100; i++)
    {
        fprintf(stderr,"%s\n",logarr[i]);

    }
    for (int i = 0; i < numprocesses; i++)
    {
        free(processes[i].name);
        for (int j = 0; j < processes[i].size; j++)
        {
            free(processes[i].arguments[j]);
        }
    }
    free(processes);
    for (int i = 0; i < *logindex; i++)
    {
        munmap(logarr[i],sizeof(char)*100);

    }
}
