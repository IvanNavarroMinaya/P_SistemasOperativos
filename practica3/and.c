#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Prototipos de funciones */
char **tokenStr(char *argv);
void executeProgram(char **str);

int
main(int argc, char *argv[])
{
	/* El enunciado pide al menos 2 argumentos para los comandos (ej: ./and cmd1 cmd2) */
	if (argc < 3) {
		fprintf(stderr, "usage: and cmd cmd [cmd ...]\n");
		exit(EXIT_FAILURE);
	}

	int status = 0;
	int ret = 0;	// Guardará el código de salida del último comando ejecutado

	for (int i = 1; i < argc; i++) {
		/* Tokenizamos el argumento actual para separar el comando de sus argumentos */
		char **str = tokenStr(argv[i]);

		/* Creamos el proceso hijo y ejecutamos el comando */
		executeProgram(str);

		/* El padre se bloquea hasta que el hijo termina para evitar procesos zombies */
		wait(&status);

		/* WEXITSTATUS extrae los 8 bits del código de retorno real */
		ret = WEXITSTATUS(status);

		/* Limpiamos memoria del comando actual antes de pasar al siguiente o salir */
		free(str[0]); // Liberamos la ruta 
		free(str);    // Liberamos el array de punteros

		/* Aplicación de la lógica tipo '&&': si un comando falla, rompemos el bucle */
		if (ret)
			break;
	}

	/* El programa termina con el status del último comando que se ejecutó */
	exit(ret);
}

char **
tokenStr(char *argv)
{
	char *token;
	char *save_ptr;

	/* Reservamos espacio para los punteros de los argumentos */
	char **str = malloc(sizeof(char *) * (64));

	if (!str) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	int i = 0;

	/* Sacamos el primer token (el nombre del comando) */
	token = strtok_r(argv, " ", &save_ptr);

	/* Reservamos memoria para construir la ruta */
	str[i] = malloc(1024);
	if (!str[i]) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* Si el usuario ya puso /bin/, lo saltamos para no duplicarlo */
	if (!strncmp(token, "/bin/", 5))
		token += 5;

	strcpy(str[i], "/bin/");
	strcat(str[i], token);

	for (i = 1; token; i++) {
		token = strtok_r(NULL, " ", &save_ptr);

		str[i] = token;	// Apuntan a trozos del argv original, no necesitan malloc
	}

	/* El último puntero de str es NULL */

	return str;
}

void
executeProgram(char **str)
{
	/* Duplicamos el proceso actual */
	pid_t pid = fork();

	switch (pid) {
	case -1:
		fprintf(stderr, "error: fork has failed");

		exit(EXIT_FAILURE);
		break;

	case 0:
		/* Reemplazamos la memoria del proceso hijo por el nuevo programa. */
		execv(str[0], str);

		/* Si llegamos aquí es que execv ha fallado */
		perror("execv");

		/* Limpiamos la memoria en el hijo antes de salir por error */
		free(str[0]);
		free(str);

		exit(EXIT_FAILURE);

	default:
		return;
	}
}
