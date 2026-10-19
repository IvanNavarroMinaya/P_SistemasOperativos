#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "stack.h"

/* Prototipos de funciones */
void *thread_func(void *arg_to_cast);
void cleanup(Stack * s);

/* Estructura para los hilos de ejecución */
struct Value {
	int id;
	int v;
};
typedef struct Value Value;

/* Estructura para los argumentos de multihilo */
struct thread_args {
	Stack *s;
	int id;
};
typedef struct thread_args thread_args;

/* Definición de constantes */
enum {
	CAPACITY = 100,
	NUM_THREADS = 100,
	ITEMS_TO_PUSH = 100,
	ITEMS_TO_POP = 40,
	ITEMS_LEFT = NUM_THREADS * (ITEMS_TO_PUSH - ITEMS_TO_POP)
};

int
main(void)
{
	/* Crear una pila con capacidad 100 */
	Stack *s = newstack(CAPACITY);

	if (!s) {
		fprintf(stderr, "Failed to create stack \n");
		exit(EXIT_FAILURE);
	}

	/* Definimos array que tendrá los id de los hilos */
	pthread_t threads_id[NUM_THREADS];

	/* Crear 100 hilos */
	for (int i = 0; i < NUM_THREADS; i++) {
		/* Creamos argumento para la función multihilo */
		thread_args *arg = malloc(sizeof(thread_args));

		arg->s = s;
		arg->id = i;

		/* Pasamos el id del hilo, NULL(valores por defecto), la función multihilo y su argumento */
		if (pthread_create(&threads_id[i], NULL, &thread_func, arg)) {
			perror("pthread_created");

			/* Limpieza de argumento por error */
			free(arg);

			/* Cerramos los hilos creados */
			for (int j = 0; j < i - 1; j++) {
				/* Llamamos a join con NULL porque no nos importa que devuelve el hilo.
                   En caso de que falle, cerramos todos los hilos posibles por limpieza.
                   Aún así luego acabamo con error */
				if (pthread_join(threads_id[j], NULL))
					perror("pthread_join");
			}

			/* Limpieza por error en create (y a lo mejor en join) */
			cleanup(s);
		}
	}

	/* Esperar a los 100 hilos */
	for (int i = 0; i < NUM_THREADS; i++) {
		/* Llamamos a join con NULL porque no nos importa que devuelve el hilo */
		void *status;

		if (pthread_join(threads_id[i], &status)) {
			perror("pthread_join");

			/* Limpieza */
			cleanup(s);
		}

		if (status)
			cleanup(s);
	}

	/* Comprobar que hay 6000 elementos */
	if (nelems(s) == ITEMS_LEFT) {
		/* Creamos array con el entero máximo por id para comprobación */
		int prev_values[NUM_THREADS];

		for (int i = 0; i < NUM_THREADS; i++)
			prev_values[i] = INT_MAX;

		/* Estraemos items restantes */
		for (int i = 0; i < ITEMS_LEFT; i++) {
			Value *value = ((Value *)pop(s));

			/* El anterior item extraido debe ser mayor al actual(no pueden ser iguales) */
			if (value->v > prev_values[value->id]) {
				fprintf(stderr,
					"Invalid value order \n");

				/* Limpieza */
				cleanup(s);
			}
			/* Nuevo valor anterior  */
			prev_values[value->id] = value->v;

			/* Libero memoria extraida */
			free(value);
		}
	} else {
		fprintf(stderr,
			"Unexpected number of remaining elements \n");

		/* Limpieza */
		cleanup(s);
	}

	freestack(s);
	exit(EXIT_SUCCESS);
}

void *
thread_func(void *arg_to_cast)
{
	/* Casteamos porque el método obliga a recibir un void * */
	thread_args *arg = (thread_args *)arg_to_cast;

	/* Definimos pila, Value y contador de mis no elementos */
	Stack *s = arg->s;
	Value *value;
	int count_non_my_elems = 0;

	/* Por cada hilo, añadir 100 valores */
	for (int i = 0; i < ITEMS_TO_PUSH; i++) {
		/* Reservamos memoria para elem de la pila */
		value = malloc(sizeof(Value));

		value->id = arg->id;
		value->v = i;

		/* Metemos el item en la pila */
		push(s, value);

		/* Volvemos porque ha fallado realloc. 
		   Si nada más falla saldremos, aun así,
		   con error, tras esperar los 100 hilos
		   y liberar la máxima memoria posible. */
		if (stackerror(s))
			return (void *)1;
	}

	/* Por cada hilo, extraer 40 valores */
	for (int i = 0; i < ITEMS_TO_POP; i++) {
		/* Extraemos item de la pila */
		Value *value_to_free = (Value *)pop(s);

		/* Si no es mi id, incremento e imprimo */
		if (value_to_free->id != arg->id) {
			count_non_my_elems++;
			printf("Counter for id (%d): %d \n", 
                    arg->id, count_non_my_elems);
		}
		/* Libero memoria del elemnto extraido */
		free(value_to_free);
	}

	free(arg);
	return NULL;
}

void
cleanup(Stack *s)
{
	/* Imprimimos estado de la pila */
	dumpstack(s);

	/* Extraemos los valores de la pila para liberarlos */
	Value *value_to_free = (Value *)pop(s);

	while (value_to_free) {
		free(value_to_free);
		value_to_free = (Value *)pop(s);
	}

	/* Liberamos pila */
	freestack(s);

	/* Salimos con error */
	exit(EXIT_FAILURE);
}
