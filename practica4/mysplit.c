#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

/* Prototipos de las funciones */
long checkArgs(char *argv, int fd_read);
void writeOnFile(char *argv, int fd_read, long bytes_to_split);
ssize_t readFromFile(int fd_read, char *buffer, long bytes_rest);
int openFileToWrite(char *argv, int i);

/* Definición de constantes */
enum {
	MAX_FILES = 999,
	FILE_NAME_SIZE = 256
};

int
main(int argc, char *argv[])
{
	/* Validación de argumentos: necesitamos el tamaño N y el nombre del fichero */
	if (argc != 3) {
		fprintf(stderr, "usage: mysplit N file\n");
		exit(EXIT_FAILURE);
	}

	/* Abrimos el fichero original en modo solo lectura */
	int fd_read = open(argv[2], O_RDONLY);

	/* Llamamos a la lógica principal: checkArgs valida N y writeOnFile hace el split */
	writeOnFile(argv[2], fd_read, checkArgs(argv[1], fd_read));

	/* Siempre cerramos los descriptores de fichero para no dejar recursos colgados */
	if (close(fd_read) == -1) {
		fprintf(stderr, "error: close has failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

long
checkArgs(char *argv_N, int fd_read)
{
	/* Verificamos si el fichero de entrada se abrió correctamente */
	if (fd_read == -1) {
		fprintf(stderr, "error: the read file couldn't be opened\n");
		exit(EXIT_FAILURE);
	}

	/* Usamos strtol en lugar de atoi porque permite detectar errores de 
	   conversión y desbordamientos (LONG_MIN/MAX) */
	char *endptr;
	long bytes_to_split = strtol(argv_N, &endptr, 10);

	/* Si endptr no apunta al final del string, es que había caracteres no numéricos */
	if (*endptr != '\0' || bytes_to_split <= 0 ||
	    bytes_to_split == LONG_MIN || bytes_to_split == LONG_MAX) {
		fprintf(stderr,
			"error: invalid byte count or strtol has failed\n");
		exit(EXIT_FAILURE);
	}

	return bytes_to_split;
}

void
writeOnFile(char *argv_file, int fd_read, long bytes_to_split)
{
	ssize_t bytes_read = 1;

	/* Bucle principal: va creando trozos numerados */
	for (int i = 0; bytes_read; i++) {
		/* Comprobamos no haber llegado al límite de archivos */
		if (i > MAX_FILES) {
			fprintf(stderr, "error: file limit reached\n");
			exit(EXIT_FAILURE);
		}
		/* Reservamos memoria para el buffer del tamaño del trozo solicitado */
		char *buffer = malloc(bytes_to_split);

		if (!buffer) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		/* Leemos el primer bloque del fichero */
		bytes_read = readFromFile(fd_read, buffer, bytes_to_split);

		/* Si bytes_read es 0, hemos llegado al final del fichero (EOF) */
		if (bytes_read == 0) {
			free(buffer);
			break;
		}

		/* Como read() puede leer menos de lo solicitado. 
		   Insistimos hasta completar el trozo o llegar al final del fichero. */
		while (bytes_read < bytes_to_split) {
			ssize_t r = readFromFile(fd_read, buffer + bytes_read,
						 bytes_to_split - bytes_read);

			if (r == 0)
				break;	// Fin de fichero antes de llenar el buffer
			bytes_read += r;
		}

		/* Creamos el nuevo fichero de salida */
		int fd_write = openFileToWrite(argv_file, i);

		/* Escribimos el contenido del buffer en el nuevo trozo */
		if (write(fd_write, buffer, bytes_read) != bytes_read) {
			fprintf(stderr, "error: write has failed\n");
			exit(EXIT_FAILURE);
		}

		/* Liberamos memoria y cerramos el fichero de escritura en cada iteración */
		free(buffer);

		if (close(fd_write) == -1) {
			fprintf(stderr, "error: close has failed\n");
			exit(EXIT_FAILURE);
		}
	}
}

ssize_t
readFromFile(int fd_read, char *buffer, long bytes_rest)
{
	/* Comprobamos si la lectura fue errónea */
	ssize_t bytes_read = read(fd_read, buffer, bytes_rest);

	if (bytes_read == -1) {
		fprintf(stderr, "error: read has failed\n");
		exit(EXIT_FAILURE);
	}

	return bytes_read;
}

int
openFileToWrite(char *argv_name, int i)
{
	char file_name[FILE_NAME_SIZE];

	/* Generamos el nombre con formato */
	snprintf(file_name, sizeof(file_name), "%03d%s", i, argv_name);

	/* Abrimos con O_CREAT (crear si no existe).
	   Permisos en octal, 0664 (rw-rw-r--) */
	int fd_write = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0664);

	if (fd_write == -1) {
		fprintf(stderr, "error: the write file couldn't be opened\n");
		exit(EXIT_FAILURE);
	}
	return fd_write;
}
