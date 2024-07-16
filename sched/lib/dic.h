#include <inc/stdio.h>


#define MAX_ENTRIES 100

// Definición de la estructura del diccionario
struct DictionaryEntry {
	int key;
	int value;
};

// Función para agregar una nueva entrada al diccionario
void
addEntry(struct DictionaryEntry dictionary[], int *count, int key)
{
	// Buscar si la clave ya existe en el diccionario
	for (int i = 0; i < *count; i++) {
		if (dictionary[i].key == key) {
			// Incrementar el valor si la clave ya existe
			dictionary[i].value++;
			// cprintf("Se ha incrementado el valor de la entrada con clave %d.\n", key);
			return;
		}
	}

	// Agregar una nueva entrada si la clave no existe
	if (*count < MAX_ENTRIES) {
		dictionary[*count].key = key;
		dictionary[*count].value = 1;
		(*count)++;
		// cprintf("Entrada con clave %d\n", key);
	} else {
		cprintf("El diccionario está lleno. No se pueden agregar más "
		        "entradas.\n");
	}
}

// Función para imprimir todo el diccionario
void
printDictionary(struct DictionaryEntry dictionary[], int count)
{
	cprintf("Número de ejecuciones por cada proceso: \n");
	for (int i = 0; i < count; i++) {
		cprintf("\tProccesId: [%08x], NumOfRuns: %d\n",
		        dictionary[i].key,
		        dictionary[i].value);
	}
}