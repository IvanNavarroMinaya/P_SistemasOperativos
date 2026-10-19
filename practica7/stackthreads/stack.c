#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "stack.h"

Stack *
newstack(int sz)
{
	/* Si el tamaño es inválido, devolver NULL */
	if (sz <= 0) {
		fprintf(stderr, "Error: Stack size must be greater than 0.\n");
		return NULL;
	}

	/* Reservar memoria para la estructura Stack */
	Stack *s = malloc(sizeof(Stack));

	if (!s) {
		perror("malloc");
		return NULL;
	}

	/* Reservar memoria para los elementos de la pila */
	s->items = malloc(sizeof(void *) * sz);
	if (!s->items) {
		perror("malloc");
		free(s);
		return NULL;
	}

	/* Inicializar los campos de la pila */
	s->size = sz;
	s->num_elems = 0;
	if (pthread_mutex_init(&s->lk, NULL)) {
		perror("pthread_mutex_init");
		free(s->items);
		free(s);
		return NULL;
	}
	s->error = 0;

	return s;
}

int
isempty(Stack *s)
{
	/* Bloquear para obtener el valor propio del hilo en la variable local */
	pthread_mutex_lock(&s->lk);
	int empty = s->num_elems;

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);

	if (!empty)
		return 1;
	else
		return 0;
}

void
push(Stack *s, void *elem)
{
	/* Establecer bloqueo para para evitar condiciones de carrera */
	pthread_mutex_lock(&s->lk);

	/* Comprobar si la pila está llena */
	if (s->num_elems == s->size) {
		/* Doblar memoria reservada para los elementos de la pila. 
		   Asignar un puntero temporal para no perder memoria */
		void **p_temp =
		    realloc(s->items, sizeof(void *) * (s->size * 2));
		if (!p_temp) {
			perror("realloc");

			/* Marcar que ha habido un error */
			s->error = 1;

			/* Abrir bloqueo */
			pthread_mutex_unlock(&s->lk);
			return;	// RESERVAR SALIR CON ERROR
		}
		s->items = p_temp;

		/* Doblar tamaño de la pila */
		s->size *= 2;
	}

	/* Añadir el nuevo elemento a la pila */
	s->items[s->num_elems] = elem;
	s->num_elems++;

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);
}

void *
pop(Stack *s)
{
	/* Establecer bloqueo para para evitar condiciones de carrera */
	pthread_mutex_lock(&s->lk);

	/* Comprobar si la pila está vacía. 
	   Importante aclarar que se usa num_elems. De usar isempty,
	   se producirián cambios de contexto, provocando segmtation fault
	   (si el candado se ponía depués, de ponerlo antes se bloquearía el mismo) */
	if (s->num_elems == 0) {
        /* Abrir bloqueo */
	    pthread_mutex_unlock(&s->lk);
        
		return NULL;
    }

	/* Quita y devuelve el último elemento de la pila */
	void *item_to_return = s->items[s->num_elems - 1];

	s->items[s->num_elems - 1] = NULL;
	s->num_elems--;

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);

	return item_to_return;
}

int
nelems(Stack *s)
{
	/* Bloquear para guardar la variable a nivel de fichero en variable local */
	pthread_mutex_lock(&s->lk);
	int n = s->num_elems;

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);

	return n;
}

void
dumpstack(Stack *s)
{
	/* Bloquear para imprimir los valores del hilo */
	pthread_mutex_lock(&s->lk);

	fprintf(stderr, "Stack capacity:                    %d\n", s->size);
	fprintf(stderr, "Number of elements in stack:       %d\n",
		s->num_elems);
	fprintf(stderr, "Stack elements (memory addresses):   \n");
	for (int i = 0; i < s->num_elems; i++) {
		fprintf(stderr, "\t- %p\n", s->items[i]);
	}

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);
}

int
stackerror(Stack *s)
{
	/* Bloquear para recoger la variable error */
	pthread_mutex_lock(&s->lk);

	int err = s->error;

	/* Volver a poner la variable de nivel fichero a false */
	s->error = 0;

	/* Abrir bloqueo */
	pthread_mutex_unlock(&s->lk);

	return err;
}

/* No se bloquea(lock) por que se llama cuando han finalizado todos los hilos.
"No se puede llamar a esta función mientras que otros hilos están usando la pila." */
void
freestack(Stack *s)
{
	/* Liberar memoria del array de la pila */
	free(s->items);

	/* Destruir el candado */
	if (pthread_mutex_destroy(&s->lk)) {
		perror("pthread_mutex_destroy");
		return;
	}

	/* Liberar memoria de la estructura Stack */
	free(s);
}
