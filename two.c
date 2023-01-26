/*
 * cc -std=gnu99 -o two two.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
 #include <sys/time.h>
#include "two.h"

const char *progname;

static void
usage(const char *msg)
{
	if (msg)
	{
		fprintf(stderr, "%s: %s\n", progname, msg);
	}
	fprintf(stderr, "usage: %s n\n", progname);
	exit(1);
}

int main(int argc, char *argv[])
{
	if ((progname = strrchr(argv[0], '/')) == NULL)
	{
		progname = argv[0];
	}
	else
	{
		progname++;
	}

	if (argc != 2)
	{
		usage(NULL);
	}

	char *notnum = NULL;
	const int n = (int)strtol(argv[1], &notnum, 10);

	if (notnum && *notnum)
	{
		usage("not a decimal number");
	}

	for (int i = 0; i < n; i++)
	{
        // struct timeval tv;
        // gettimeofday(&tv,NULL);
        // unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
		printf("%*d\n", 8 * n, n);
		fflush(stdout);
		sleep(1);
	}
	return 0;
}
