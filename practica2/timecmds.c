#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

pid_t executeProgram(char **str);
char **tokenStr(char *argv);
int waitPrograms(int argc, pid_t *pidArray, char **commandArray,
		 time_t initialTime);

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "error: not enought arguments");

		exit(EXIT_FAILURE);
	}

	pid_t *pidArray = malloc(sizeof(pid_t) * (argc - 1));
	char **commandArray = malloc(sizeof(char *) * (argc - 1));

	if (commandArray == NULL || pidArray == NULL) {
		fprintf(stderr, "error: malloc has failed");

		exit(EXIT_FAILURE);
	}

	time_t initialTime;

	time(&initialTime);

	for (int i = 1; i <= argc - 1; i++) {
		char **str = tokenStr(argv[i]);
		commandArray[i - 1] = strdup(str[0]);

		pidArray[i - 1] = executeProgram(str);

		free(str);
	}

	int ret = waitPrograms(argc, pidArray, commandArray, initialTime);

	free(pidArray);

	for (int i = 0; i < argc - 1; i++) {
		free(commandArray[i]);
	}
	free(commandArray);

	exit(ret);
}

char **
tokenStr(char *argv)
{
	char *token;
	char *saved_str;
	char **str = malloc(sizeof(char *) * (11));
	int i = 0;

	token = strtok_r(argv, " ", &saved_str);

	for (i = 0; token; ++i) {
		str[i] = token;

		token = strtok_r(NULL, " ", &saved_str);
	}
	str[i] = NULL;

	return str;
}

pid_t
executeProgram(char **str)
{
	pid_t pid = fork();

	switch (pid) {
	case -1:
		fprintf(stderr, "error: fork has failed");

		exit(EXIT_FAILURE);
		break;

	case 0:
		execv(str[0], str);

		perror("execv");

		exit(EXIT_FAILURE);
		break;
	}

	return pid;
}

int
waitPrograms(int argc, pid_t *pidArray, char **commandArray, time_t initialTime)
{
	int ret = EXIT_SUCCESS;

	for (int i = 0; i <= argc - 2; i++) {
		pid_t pid;
		int status;
		time_t endTime;

		pid = wait(&status);

		time(&endTime);

		for (int j = 0; j <= argc - 2; j++) {
			if (pid == pidArray[j]) {
				if (!status) {
					printf
					    ("cmd:%s, pid: %d, time: %ld, status: success\n",
					     commandArray[j], pid,
					     endTime - initialTime);
				} else {
					printf
					    ("cmd:%s, pid: %d, time: %ld, status: failure\n",
					     commandArray[j], pid,
					     endTime - initialTime);

					ret = EXIT_FAILURE;
				}
				break;
			}
		}
	}
	return ret;
}
