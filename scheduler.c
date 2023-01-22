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
void printprocesses(process* p, int numprocesses)
{
    for (int i = 0; i<numprocesses; i++)
    {
        fprintf(stderr, "%s ",p[i].name);
        for (int j = 0; j<p[i].size; j++){
            fprintf(stderr, "%s ",p[i].arguments[j]);
        }
        fprintf(stderr, "\n");
    }
}
int main(int argc, char *argv[])
{
    // array for keeping track of processes
    process* processes= (process *)malloc(sizeof(process));

    // usage error checking
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
    // parse and add all command line arguments to process array
    while(index<argc)
    {   
    
        process p;

        int size = 0;
        p.name=(char*)malloc(sizeof(char)*(strlen(argv[index])+1));
        strcpy(p.name,argv[index]);
        numprocesses++;
        index++;
        // delimiter for separating each process and their arguments
        while(index<argc && strcmp(argv[index],":")!=0){
            p.arguments[size]=(char*)malloc(sizeof(char)*(strlen(argv[index])+1));
            strcpy(p.arguments[size],argv[index]);
            size++;
            index++;
        }
        
        p.size=size;
        size=0;
        // extend process array size upon appending process
        processes[numprocesses-1]=p;
        
    
        process* reallocproccess= realloc(processes,(numprocesses+1) * sizeof(process));
        if (reallocproccess) {
         processes = reallocproccess;
        }
        else {
            fprintf(stderr, "ERROR: allocating memory for process details");

        }
        if(index<argc && strcmp(argv[index],":")==0){
            index++;
        }
   } 
   printprocesses(processes,numprocesses);

   for (int i = 0; i<numprocesses; i++)
    {
        free(processes[i].name);
        for (int j = 0; j<processes[i].size; j++){
            free(processes[i].arguments[j]);
        }
    }
     free(processes);


}

