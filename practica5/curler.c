#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* Prototipos de funciones */
FILE *getStream(int argc, char *argv);
char *getUrl(FILE * stream, int err_count);
void checkUrl(FILE * stream, char *url);
char **createExecutionString(FILE * stream, char *url);
void executeProgram(FILE * stream, char **exec_str);
void redirectOutput(FILE * stream, char **exec_str);
void cleanup(FILE * stream, char *str);

/* Definición de constantes */
enum {
	MAX_URL_SIZE = 512,
	STANDARD_ERROR = 255
};

int
main(int argc, char *argv[])
{
	/* Validación inicial de argumentos */
	if (argc > 2) {
		fprintf(stderr, "error: too many arguments\n");
		exit(STANDARD_ERROR);
	}

	int err_count = 0;

	/* Abrimos el flujo de entrada una sola vez (fichero o stdin) */
	FILE *stream = getStream(argc, argv[1]);

	while (1) {
		/* Bucle principal: obtener URL, crear argumentos y ejecutar */
		executeProgram(stream, createExecutionString
			       (stream, getUrl(stream, err_count)));

		int status = 0;

		/* El padre espera al hijo para evitar procesos zombie y recoger su estado */
		wait(&status);

		/* Si el proceso terminó correctamente y curl devolvió error, sumamos al contador */
		if (WIFEXITED(status))
			if (WEXITSTATUS(status) && err_count < 255)
				err_count++;
	}
}

FILE *
getStream(int argc, char *argv)
{
	FILE *stream;

	/* Seleccionamos la fuente de entrada según los argumentos pasados */
	if (argc == 2)
		stream = fopen(argv, "r");
	else
		stream = stdin;

	if (stream == NULL) {
		fprintf(stderr, "error: get stream has failed\n");
		exit(STANDARD_ERROR);
	}

	return stream;
}

char *
getUrl(FILE *stream, int ERR_COUNT)
{
	char *url = malloc(MAX_URL_SIZE + 2);

	/* Leemos una línea del flujo; el tamaño MAX_URL_SIZE + 2 permite detectar si sobra texto */
	if (fgets(url, MAX_URL_SIZE + 2, stream) == NULL) {
		if (feof(stream)) {
			/* Liberamos memoria, cerramos flujo y salimos con el contador de fallos */
			cleanup(stream, url);
			exit(ERR_COUNT);
		} else {
			fprintf(stderr, "error: fgets has failed\n");

			cleanup(stream, url);
			exit(STANDARD_ERROR);
		}
	}

	checkUrl(stream, url);

	/* Eliminamos el carácter de salto de línea si existe al final de la cadena */
	if (url[strlen(url) - 1] == '\n')
		url[strlen(url) - 1] = '\0';

	return url;
}

void
checkUrl(FILE *stream, char *url)
{
	/* Validamos si la URL es demasiado larga basándonos en si fgets llenó el buffer sin encontrar \n */
	if (strlen(url) == MAX_URL_SIZE + 1 && url[strlen(url) - 1] != '\n') {
		fprintf(stderr, "error: URL too long\n");

		cleanup(stream, url);
		exit(STANDARD_ERROR);
	} else if (strncmp(url, "http://", 7) && strncmp(url, "https://", 8)) {
		/* Error si la línea no comienza con los protocolos permitidos */
		fprintf(stderr, "error: invalid line %s", url);

		cleanup(stream, url);
		exit(STANDARD_ERROR);
	}
}

char **
createExecutionString(FILE *stream, char *url)
{
	/* Reservamos memoria para el array de punteros de execv (curl, timeout, 5, url, NULL) */
	char **str = malloc(sizeof(char *) * 5);

	if (!str) {
		perror("malloc");

		cleanup(stream, url);
		exit(STANDARD_ERROR);
	}

	str[0] = "curl";
	str[1] = "--connect-timeout";
	str[2] = "5";
	str[3] = strdup(url);	/* Duplicamos la URL para independizarla del buffer de lectura */
	if (!str[3]) {
		perror("malloc");

		cleanup(stream, url);
		free(str);
		exit(STANDARD_ERROR);
	}

	free(url);		/* Liberamos el buffer original tras la duplicación */
	str[4] = NULL;		/* execv requiere que el último elemento sea NULL */

	return str;
}

void
executeProgram(FILE *stream, char **exec_str)
{
	pid_t pid = fork();

	switch (pid) {
	case -1:
		fprintf(stderr, "error: fork has failed\n");

		cleanup(stream, exec_str[3]);
		free(exec_str);
		exit(STANDARD_ERROR);
		break;

	case 0:		/* Proceso hijo */
		redirectOutput(stream, exec_str);
		execv("/usr/bin/curl", exec_str);

		/* Solo llegamos aquí si execv falla: liberamos recursos y salimos */
		perror("execv");

		cleanup(stream, exec_str[3]);
		free(exec_str);
		exit(STANDARD_ERROR);

	default:		/* Proceso padre */
		/* El padre libera la memoria de los argumentos del comando tras el fork */
		free(exec_str[3]);
		free(exec_str);
		return;
	}
}

void
redirectOutput(FILE *stream, char **exec_str)
{
	/* Abrimos /dev/null para que curl no escriba nada en pantalla */
	int fd_null = open("/dev/null", O_WRONLY);

	if (fd_null == -1) {
		/* Si falla la apertura, limpiamos todo antes de salir */
		cleanup(stream, exec_str[3]);
		free(exec_str);
		exit(STANDARD_ERROR);
	}

	/* Redirigimos la salida estándar y la de errores al descriptor de /dev/null */
	dup2(fd_null, STDOUT_FILENO);
	dup2(fd_null, STDERR_FILENO);

	if (close(fd_null) == -1) {
		cleanup(stream, exec_str[3]);
		free(exec_str);
		exit(STANDARD_ERROR);
	}
}

void
cleanup(FILE *stream, char *str)
{
	/* Función centralizada para liberar el buffer de texto y cerrar el flujo de entrada */
	free(str);
	if (stream != stdin)
		fclose(stream);
}
