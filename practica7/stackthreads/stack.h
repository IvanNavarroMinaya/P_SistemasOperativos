/* Estructura para representar una pila */
struct Stack {
	void **items;		// Array de punteros a elementos
	int size;		// Capacidad total de la pila
	int num_elems;		// Número de elementos actualmente en la pila
	pthread_mutex_t lk;	// Candado para sincronización

	int error;		// Variable para saber si en stack hubo un error
};

/* Tipo para representar una pila */
typedef struct Stack Stack;

/* Crea una nueva pila */
Stack *newstack(int sz);

    /* Pila nueva de tamaño sz > 0
       En caso de error, devuelve NULL. */

/* Indica si la pila está vacía */
int isempty(Stack *s);

    /* Retorna true si la pila está vacía, 
       false en caso contrario. */

/* Añade un elemento a la pila*/
void push(Stack *s, void *elem);

    /* Si la pila está llena,
       hace un realloc para doblar su tamaño. */

/* Quita un elemento de la pila*/
void *pop(Stack *s);

    /* No es necesario reducir el tamaño.
       Si la pila está vacía, devuelve NULL. */

/* Retorna el número de elementos en la pila */
int nelems(Stack *s);

/* Imprime el contenido de la pila */
void dumpstack(Stack *s);

    /* Se imprime por salida de error:
       - Capacidad total de la pila
       - Número de elementos en la pila
       - Elementos de la pila (direcciones de memoria) */

/* Devuelve si ha habido un error */
int stackerror(Stack *s);

    /* Vuelve a poner la variable de error a false
       se esta manera solo un hilo se encarga de limpiar */

/* Libera toda la memoria de la pila */
void freestack(Stack *s);

    /* No se puede liberar la memoria si
       la pila está siendo utilizada por otro hilo. 
       Solo libera los recursos propios de la pila. */
