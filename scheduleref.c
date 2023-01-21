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

int *childp;
char **children;
int *count;
int *pathsize;
char *result;
pid_t *pid;
time_t *T;
time_t *Ta;
char path[1000];
bool *timer;
bool end;
void printstat(struct stat sb);
char *find(char *file, char *d, char *f, bool s, bool *, int *, char *);
char *findtext(char *file, char *d, char *f, bool s, bool *, int *, char *);
void listchildren();
void killChild(int i);
int fd[2];

void sigh()
{
    dup2(fd[0], STDIN_FILENO);
    end = true;
}
int main()
{
    signal(SIGUSR1, sigh);
    childp = mmap(NULL, sizeof(int) * 10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    pathsize = mmap(NULL, sizeof(int) * 10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    memset(&(childp[0]), 0, 10);
    count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    children = mmap(NULL, sizeof(char *) * 10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int i = 0; i < 10; i++)
    {
        children[i] = mmap(NULL, sizeof(char) * 100, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    }
    int pid = getpid();
    T = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    Ta = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    timer = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    *timer = false;
    int realstdin = dup(STDIN_FILENO);
    pipe(fd);
    *pathsize = 1000;
    while (true)
    {

        getcwd(path, 1000);
        fprintf(stderr, "\033[0;34mfindstuff %s\033[0;37m$ ", path);

        char filename[10000];
        char command[1000];
        char arg[1000];
        char file[1000];

        memset(&(filename[0]), 0, 10000);

        read(STDIN_FILENO, filename, 10000);

        if (end == true)
        {

            *count = *count - 1;
            fprintf(stderr, "\n%s\n", filename);
            end = false;
            dup2(realstdin, STDIN_FILENO);
            continue;
        }
        else
        {
            filename[strlen(filename)] = '\0';
        }
        //close(fd[0]);
        if (strchr(filename, '\n'))
        {
            char *chg = strchr(filename, '\n');
            *chg = '\0';
        }
        memcpy(command, &filename[0], 4);
        command[4] = '\0';
        if (strcmp(command, "find") == 0)
        {

            char *p = strchr(filename, ' ');
            int i;
            if (p)
            {
                i = p - filename;
            }
            p++;
            char *s = strchr(p, ' ');
            if (s)
            {
                i = s - p;
                memset(&(file[0]), 0, 1000);
                memcpy(file, &filename[4], 1);
                memcpy(file, &filename[5], i);
                int size = i;
                if (strstr(filename, "-f") || strstr(filename, "-s"))
                {
                    i = strlen(filename) - 4 - i;
                    memset(&(arg[0]), 0, 1000);
                    memcpy(arg, &filename[5 + size], 1);
                    memcpy(arg, &filename[5 + size + 1], i);
                    arg[i] = '\0';
                }
            }
            else
            {
                i = strlen(p);
                memset(&(file[0]), 0, 1000);
                memcpy(file, &filename[4], 1);
                memcpy(file, &filename[5], i);
                memset(&(arg[0]), 0, 1000);
            }

            if (fork() == 0)
            {
                clock_t begin = clock();

                *count = *count + 1;

                int index = -1;
                for (int j = 0; j < 10; j++)
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
                char job[100];
                job[0] = '\0';

                strcat(job, " Finding ");
                strcat(job, file);

                char *paths = (char *)malloc(sizeof(char) * 100);

                bool found = false;
                bool s = false;
                if (strstr(arg, "-s"))
                {
                    s = true;
                    strcat(job, " in current directory and sub-directories ");
                }
                else
                {
                    strcat(job, " in current directory ");
                }
                if (file[0] == '\"')
                {

                    char *i = strchr(file, '\"');
                    i++;
                    char *f = strchr(i, '\"');
                    int size = f - i;

                    char search[1000];
                    memcpy(search, i, size);
                    search[size] = '\0';

                    if (strstr(arg, "-f"))
                    {
                        strcat(job, "for files with extention . ");

                        i = strchr(arg, ':');
                        i++;
                        char *e = strchr(i, ' ');
                        size = e - i;
                        char ext[1000];
                        memcpy(ext, i, size);
                        strcat(job, ext);
                        for (int j = 0; j < strlen(job); j++)
                        {
                            children[index][j] = job[j];
                        }
                        paths = findtext(search, ".", ext, s, &found, fd, paths);
                    }
                    else
                    {
                        strcat(job, "for all file types");
                        for (int j = 0; j < strlen(job); j++)
                        {
                            children[index][j] = job[j];
                        }
                        paths = findtext(search, ".", " ", s, &found, fd, paths);
                    }
                }
                else
                {
                    for (int j = 0; j < strlen(job); j++)
                    {
                        children[index][j] = job[j];
                    }
                    paths = find(file, ".", " ", s, &found, fd, paths);
                }
                clock_t end = clock();
                clock_t duration = (double)(end - begin) / CLOCKS_PER_SEC;
                double time_spent = (double)(end - begin) / (CLOCKS_PER_SEC / 1000);
                int h = (time_spent / 3600);
                char hrs[4];
                sprintf(hrs, "%d", h);
                int m = (time_spent - (3600 * h)) / 60;
                char min[4];
                sprintf(min, "%d", m);
                int sec = (time_spent - (3600 * h) - (m * 60));
                char secs[4];
                sprintf(secs, "%d", sec);
                int ms = (time_spent - (3600 * 1000 * h) - (m * 1000 * 60) - (s * 1000));
                ms = ms / -1000;
                char mill[4];
                sprintf(mill, "%d", ms);

                char timeE[10];
                strcat(timeE, hrs);
                strcat(timeE, ":");
                strcat(timeE, min);
                strcat(timeE, ":");
                strcat(timeE, secs);
                strcat(timeE, ":");
                strcat(timeE, mill);

                close(fd[0]);

                if (found == false)
                {
                    //fprintf(stderr, "Nothing Found\n");
                    write(fd[1], "Nothing Found\n", 20);
                    write(fd[1], "Time Elapsed: ", 14);
                    write(fd[1], timeE, strlen(timeE));
                }
                else
                {
                    *pathsize = strlen(paths);
                    write(fd[1], paths, strlen(paths));
                    write(fd[1], "Time Elapsed: ", 14);
                    write(fd[1], timeE, strlen(timeE));
                }
                kill(pid, SIGUSR1);
                close(fd[1]);
                memset(&(children[index][0]), 0, 100);
                childp[index] = 0;
                free(paths);
                return 0;
            }
        }
        else if (strcmp(command, "list") == 0)
        {
            listchildren();
        }
        else if (strcmp(command, "kill") == 0)
        {
            char c = filename[5];
            int x = c - '0';
            killChild(x);
        }
        else if (strcmp(command, "q") == 0 || strcmp(command, "quit") == 0)
        {
            return 0;
        }
    }
}

char *find(char *file, char *d, char *f, bool s, bool *found, int *fd, char *paths)
{

    struct dirent *dp;
    DIR *dir = opendir(d);

    if (!dir)
    {
    }
    else
        while ((dp = readdir(dir)) != NULL)
        {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
            {
                char *pat = (char *)malloc(sizeof(char) * 100);
                if (strcmp(d, ".") == 0)
                {
                    int size = 100;
                    while (getcwd(pat, size) == NULL)
                    {
                        pat = (char *)realloc(pat, size * 2);
                        size *= 2;
                    };
                }
                else
                {
                    pat = (char *)realloc(pat, (strlen(path) + strlen(d)) * 2);
                    strcat(pat, d);
                }

                strcat(pat, "/");
                strcat(pat, dp->d_name);
                if (strcmp(dp->d_name, file) == 0)
                {
                    paths = (char *)realloc(paths, sizeof(char) * (strlen(paths) + strlen(path) * 2));
                    strcat(paths, pat);
                    strcat(paths, "\n");
                    *found = true;
                }
                if (s)
                    paths = find(file, pat, f, s, found, fd, paths);
            }
        }

    closedir(dir);
    return paths;
}
char *findtext(char file[], char *d, char *f, bool s, bool *found, int *fd, char *paths)
{

    struct dirent *dp;
    DIR *dir = opendir(d);

    if (!dir)
    {
    }
    else
        while ((dp = readdir(dir)) != NULL)
        {

            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)

            {

                char *pat = (char *)malloc(sizeof(char) * 100);
                memset(&(pat[0]), 0, 100);
                if (strcmp(d, ".") == 0)
                {

                    int size = 100;
                    while (getcwd(pat, size) == NULL)
                    {
                        pat = (char *)realloc(pat, size * 2);
                        size *= 2;
                    };
                }
                else
                {
                    pat = (char *)realloc(pat, (strlen(path) + strlen(d)) * 2);
                    strcat(pat, d);
                }

                strcat(pat, "/");
                strcat(pat, dp->d_name);
                if (dp->d_type != DT_DIR)
                {
                    FILE *in_file = fopen(pat, "r");
                    if (in_file != NULL)
                    {
                        char string[50];
                        if (fgets(string, sizeof(string), in_file) == NULL)
                        {
                            break;
                        }
                        while (fgets(string, sizeof(string), in_file) == &string[0])
                        {

                            if (strstr(string, file) != 0)
                            {
                                break;
                            }
                        }

                        if (strstr(string, file) != 0)
                        {
                            char ext[10];
                            memset(&(ext[0]), 0, 10);

                            strcat(ext, ".");
                            strcat(ext, f);

                            if (strstr(dp->d_name, ext) != 0)
                            {
                                paths = (char *)realloc(paths, sizeof(char) * (strlen(paths) + strlen(path)) * 2);
                                //fprintf(stderr, "%s\n", pat);
                                strcat(paths, pat);
                                strcat(paths, "\n");
                                *found = true;
                            }
                            else if (strcmp(f, " ") == 0)
                            {
                                char *p = paths;
                                paths = (char *)realloc(paths, sizeof(char) * (strlen(paths) + strlen(path)) * 2);

                                strcat(paths, pat);
                                strcat(paths, "\n");
                                *found = true;
                            }
                        }
                    }
                    fclose(in_file);
                }

                if (s)
                    paths = findtext(file, pat, f, s, found, fd, paths);
                free(pat);
            }
        }

    closedir(dir);
    return paths;
}
void listchildren()
{
    int c = 0;

    for (int i = 0; i < 10; i++)
    {
        char *t = children[i];

        if (childp[i] != 0)
        {
            c++;
            printf("child %d pid:%d %s\n", c, childp[i], children[i]);
        }
    }
}
void killChild(int i)
{
    printf("Killing child %d pid:%d\n", i, childp[i - 1]);
    *count = *count - 1;
    kill(SIGKILL, childp[i - 1]);
    memset(&(children[i - 1][0]), 0, 100);
    childp[i - 1] = 0;
}