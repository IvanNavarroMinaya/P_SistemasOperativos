/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

char charToUpper (char c);

void removeNonChars (char *argv[], int argc);

void insertionSort (char *str[], int n, int i);

void removeDuplicates (char *str[], int n);

int
main (int argc, char *argv[])
{
  if (argc <= 1)
    {
      perror ("error: no arguments");

      exit (EXIT_FAILURE);
    }

  removeNonChars (&argv[0], argc);

  insertionSort (&argv[1], argc - 1, 1);

  removeDuplicates (&argv[1], argc - 1);

  for (int i = 1; argv[i]; i++)
    {
      printf ("%s\n", argv[i]);
    }

  exit (EXIT_SUCCESS);

}

/* 
Comprueba si el parametro es un caracter. Lo devuelve en
mayusculas en caso cierto y un 0, si no. 
*/
char
charToUpper (char c)
{
  if (isalpha ((unsigned char) c))
    return toupper (c);
  else
    return '0';
}

void
removeNonChars (char *argv[], int argc)
{
  for (int i = 1; i < argc; i++)
    {
      for (int j = 0; j < strlen (argv[i]); j++)
	{
	  argv[i][j] = charToUpper (argv[i][j]);

	  if (argv[i][j] == '0')
	    {
	      for (int k = j; argv[i][k]; k++)
		{
		  argv[i][k] = argv[i][k + 1];
		}
	      j--;
	    }
	}
    }
}

/*
Se plantea el algoritmo de ordenacióon insertion 
para ordenar las palabras de menor a mayor. Se 
entienden las menores como las primeras alfabeticamente.
*/
void
insertionSort (char *str[], int n, int i)
{
  // Se retorna en caso de que se vaya a leer la misma posición
  if (i == n)
    return;

  char *key = str[i];
  int j = i - 1;

  while (j >= 0 && strcmp (str[j], key) > 0)
    {
      str[j + 1] = str[j];
      j--;
    }
  str[j + 1] = key;

  // Se llama recursivamente, comparando la siguiente posición
  insertionSort (str, n, i + 1);
}

/*
Se recorre el array, comprobando las posiciones y eliminando 
duplicados.
*/
void
removeDuplicates (char *str[], int n)
{
  if (n == 0)
    return;

  int aux = 0;
  int k = 1;

  for (int i = 1; i < n; i++)
    {

      if (strcmp (str[i], str[k - 1]) != 0)
	{
	  str[k++] = str[i];
	  aux++;
	}
    }

  str[aux + 1] = 0;
}
