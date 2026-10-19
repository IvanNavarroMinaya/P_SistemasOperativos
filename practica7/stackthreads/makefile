# Variables para el compilador y banderas
CC = gcc
CFLAGS = -Wall -Wshadow -Wvla -g
LDFLAGS = -pthread

# Nombre del ejecutable final
TARGET = stackthreads

# Objetos necesarios
OBJS = main.o stack.o

# Compila el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Ficheros objeto
main.o: main.c stack.h
	$(CC) $(CFLAGS) -c main.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c

# Limpiar los archivos temporales y el ejecutable
clean:
	rm -f $(TARGET) $(OBJS)
