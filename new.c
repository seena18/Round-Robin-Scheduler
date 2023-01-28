#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include "parse.h"

int current_pid;

void print_args(char **args)
{
    char *current = args[0];
    int i = 0;
    while (current != NULL)
    {
        current = args[i++];
        printf("%s, ", current);
    }
    printf("\n");
}

int process_total(node **node_list)
{
    int i = 0;
    while (node_list[i] != NULL)
    {
        i++;
    }
    return i;
}

void print_process_ids(node **node_list)
{
    int i = 0;
    while (node_list[i] != NULL)
    {
        printf("pid: %i\n", node_list[i++]->pid);
    }
}

/* 
fork_all iterates through the list of all process nodes
it forks a child for each node, and stops each child process immediately
the parent process sets the pid associated with the child to its value
returns the last node index that was succefully forked
*/
int fork_all(node **node_list)
{
    int i = 0;
    int status;
    pid_t pid;
    node *current_node;
    while ((current_node = node_list[i]) != NULL)
    {
        pid = fork();
        if (pid < 0)
        {
            return i;
        }
        else if (pid == 0)
        {
            raise(SIGSTOP);
            // printf("execcing child process %s\n", current_node->name);
            execv(current_node->name, current_node->args);
            // do {
            //     wait_val = waitpid(current->pid, &status, WUNTRACED);
            // }while(wait_val == -1 && errno== EINTR);
        }
        else
        {
            current_node->pid = pid;
            waitpid(pid, &status, WUNTRACED);
        }
        i++;
    }
    return i;
}

void handler(int signum)
{
    //sigalrm 14 sigchld 17
    if (current_pid)
    {
        kill(current_pid, SIGSTOP);
    }
}

int main(int argc, char *argv[])
{
    int last_fork, total_processes, i, completed_count, status, wait_val;
    int quantum = atoi(argv[1]) * 1000;
    int sec = quantum / 1000000;
    int usec = quantum % 1000000;
    // printf("quantum sec:%i usec: %i\n", sec, usec);
    sigset_t mask;
    struct itimerval timer;
    int which = ITIMER_REAL;
    (void)sigemptyset(&mask);
    node *current;
    //create all node objects with with names, args, pid = 0, completed = 0
    node **node_list = create_nodes(argc, argv);
    total_processes = process_total(node_list);
    //initialize and stop all processes
    last_fork = fork_all(node_list);
    signal(SIGALRM, handler);
    i = 0;
    completed_count = 0;
    while (completed_count != total_processes)
    {
        if (i >= last_fork)
        {
            i = 0;
        }
        current = node_list[i];
        if (!(current->completed))
        {
            current_pid = current->pid;
            timer.it_value.tv_sec = sec;
            timer.it_value.tv_usec = usec;
            timer.it_interval.tv_sec = 0;
            timer.it_interval.tv_usec = 0;
            if (setitimer(which, &timer, NULL) < 0)
            {
                perror("timer");
                exit(EXIT_FAILURE);
            };
            kill(current->pid, SIGCONT);
            do
            {
                wait_val = waitpid(current->pid, &status, WUNTRACED);
            } while (wait_val == -1 && errno == EINTR);
            if (WIFEXITED(status))
            {
                current->completed = 1;
                completed_count++;
            }
        }
        i++;
    }
    return 0;
}