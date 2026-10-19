#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <err.h>

/* Prototipos de funciones */
int recursiveDirRead(const char *dir_path, FILE * file_to_write);
int readFile(const char *file_path, FILE * file_to_write);
int writeOnFile(FILE * file_to_write, char *buffer, size_t items_to_write);
int closeFile(FILE * file);
int closeDir(DIR * dir);

/* Definición de constantes */
enum {
	MAX_ITEMS_TO_READ = 4096,
	PATH_MAX_CRCUR = 4096
};

int
main(int argc, char *argv[])
{
	/* Validación de argumentos: el programa requiere la ruta de salida */
	if (argc != 2)
		errx(EXIT_FAILURE, "error: invalid number of arguments.");

	/* Apertura en modo binario "wb" para tratar cualquier contenido (texto o binario) */
	FILE *file_to_write = fopen(argv[1], "wb");

	if (!file_to_write) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	/* Lanzamos el recorrido recursivo empezando desde el directorio actual "." */
	if (recursiveDirRead(".", file_to_write) == -1) {
		closeFile(file_to_write);
		exit(EXIT_FAILURE);
	}

	/* Cierre limpio del fichero de salida */
	if (closeFile(file_to_write) == -1)
		exit(EXIT_FAILURE);

	exit(EXIT_SUCCESS);
}

int
recursiveDirRead(const char *dir_path, FILE *file_to_write)
{
	/* Abrimos el directorio para lectura */
	DIR *actual_dir = opendir(dir_path);

	if (!actual_dir) {
		perror("opendir");
		return -1;
	}

	struct dirent *dir_to_treat;

	errno = 0;

	/* Iteramos sobre todas las entradas del directorio */
	while ((dir_to_treat = readdir(actual_dir)) != NULL) {
		/* Saltamos los enlaces simbólicos de directorio actual y padre */
		if (strcmp(dir_to_treat->d_name, ".") &&
		    strcmp(dir_to_treat->d_name, "..")) {
			struct stat st;

			/* Construcción dinámica de la ruta completa usando snprintf para evitar desbordamientos */
			char new_path[PATH_MAX_CRCUR];

			if (snprintf
			    (new_path, sizeof(new_path), "%s/%s", dir_path,
			     dir_to_treat->d_name) >= sizeof(new_path)) {
				fprintf(stderr, "path too long\n");
				closeDir(actual_dir);
				return -1;
			}

			/* lstat permite ver el tipo de fichero sin seguir enlaces simbólicos peligrosos */
			if (lstat(new_path, &st) == -1) {
				perror("lstat");
				closeDir(actual_dir);
				return -1;
			}

			/* Si es un directorio, recursión controlada */
			if (S_ISDIR(st.st_mode)) {
				if (recursiveDirRead(new_path, file_to_write) ==
				    -1) {
					closeDir(actual_dir);
					return -1;
				}
			}
			/* Si es un fichero regular y termina en .txt, procedemos a leer */
			else if (S_ISREG(st.st_mode)) {
				size_t len = strlen(dir_to_treat->d_name);

				if (len >= 4
				    && !strcmp(dir_to_treat->d_name + len - 4,
					       ".txt")) {
					if (readFile(new_path, file_to_write) ==
					    -1) {
						closeDir(actual_dir);
						return -1;
					}
				}
			}
		}
	}

	/* Verificación de errores durante la lectura del directorio */
	if (!errno) {
		return closeDir(actual_dir);
	} else {
		perror("readdir");
		closeDir(actual_dir);
		return -1;
	}
}

int
readFile(const char *file_path, FILE *file_to_write)
{
	/* Apertura en modo "rb" (binario) para total seguridad con cualquier tipo de archivo */
	FILE *file_to_read = fopen(file_path, "rb");

	if (!file_to_read) {
		perror("fopen");
		return -1;
	}

	/* Lectura en bloques usando el buffer de 4KB para eficiencia */
	char buffer[MAX_ITEMS_TO_READ];
	size_t items_read = fread(buffer, 1, sizeof(buffer), file_to_read);

	while (items_read > 0) {
		if (writeOnFile(file_to_write, buffer, items_read) == -1) {
			closeFile(file_to_read);
			return -1;
		}
		items_read = fread(buffer, 1, sizeof(buffer), file_to_read);
	}

	/* Comprobación de errores de lectura tras salir del bucle */
	if (ferror(file_to_read)) {
		perror("fread");
		closeFile(file_to_read);
		return -1;
	}

	return closeFile(file_to_read);
}

int
writeOnFile(FILE *file_to_write, char *buffer, size_t items_to_write)
{
	/* Escritura directa en el fichero, el modo "w" o "a" depende de cómo se abrió */
	if (fwrite(buffer, 1, items_to_write, file_to_write) != items_to_write) {
		perror("fwrite");
		return -1;
	}
	return 0;
}

int
closeFile(FILE *file)
{
	if (fclose(file) == -1) {
		perror("fclose");
		return -1;
	}
	return 0;
}

int
closeDir(DIR *dir)
{
	if (closedir(dir) == -1) {
		perror("closedir");
		return -1;
	}
	return 0;
}
