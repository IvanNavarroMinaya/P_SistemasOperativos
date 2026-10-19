#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

void searchWords(char *fgrep_args[]);

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "error: not enought arguments");

		exit(EXIT_FAILURE);
	}

	char **fgrep_args = malloc(sizeof(char *) * (argc * 2));

	if (fgrep_args == NULL) {
		fprintf(stderr, "error: malloc has failed");

		exit(EXIT_FAILURE);
	}

	fgrep_args[0] = "fgrep";
	fgrep_args[1] = "-n";

	int j = 2;

	for (int i = 1; i <= argc - 2; i++) {
		fgrep_args[j++] = "-e";
		fgrep_args[j++] = argv[i];
	}

	fgrep_args[j++] = argv[argc - 1];
	fgrep_args[j] = NULL;

	searchWords(fgrep_args);

	free(fgrep_args);

	exit(EXIT_SUCCESS);
}

void
searchWords(char *fgrep_args[])
{
	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "error: fork has failed");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		execv("/usr/bin/fgrep", fgrep_args);

		perror("execv");

		exit(EXIT_FAILURE);
	} else {
		return;
	}
}
