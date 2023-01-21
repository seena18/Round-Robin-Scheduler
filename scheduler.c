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

#define MAX_ARGUMENTS 10
#define MAX_PROCESSES 101
int isNumber(char s[])
{
    for (int i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
              return 0;
    }
    return 1;
}

typedef struct process{
    char* name;
    int size;
    char* arguments[MAX_ARGUMENTS];
}process;

int main(int argc, char *argv[])
{
    process* processes= (process *)malloc(sizeof(process));
    if( argc < 3){
        fprintf(stderr, "Usage: schedule [milliseconds] [prog1 [prog1 args...]] [prog2 ...] ...\n");
        return 0;
    }
    if(!isNumber(argv[1])){
        fprintf(stderr, "Second argument should be a numerical value representing milliseconds\n");
        return 0;
    }

    int index = 2;
    int numprocesses=0;
    while(index<argc)
    {   
        process* p = (process* )malloc(sizeof(process));
        int size = 0 ;
        p->name=(char*)malloc(sizeof(char)*strlen(argv[index]));
        p->name=argv[index];
        numprocesses++;
        index++;
        while(strcmp(argv[index],":")!=0 && index<argc){
            p->arguments[size]=(char*)malloc(sizeof(char)*strlen(argv[index]));
            p->arguments[size]=argv[index];
            size++;
            index++;
        }
        p->size=size;
        size=0;
        process* reallocproccess= realloc(processes,numprocesses*sizeof(process));
        if (reallocproccess) {
         processes = reallocproccess;
        }
        else {
            fprintf(stderr, "ERROR: allocating memory for process details");

        }
   } 


}

