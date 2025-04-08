#define _CRT_SECURE_NO_WARNINGS
#define INITIAL_CAP 256
#include "pq.h"
#include <stdlib.h>
#include <stdio.h>


/* Crea la cola de prioridad PQ e inicializa sus atributos
retorna un puntero a la cola de prioridad 
retorna NULL si hubo error*/
PQ pq_create() {
	// crear el PQ y verificar su creacion
	PQ pq = (PQ)malloc(sizeof(struct Heap));
	if (pq == NULL) return NULL;

	// crear el array y verificar su creacion
	PrioValue *arr = malloc(sizeof(struct _PrioValue) * INITIAL_CAP);
	if (arr == NULL) {
		free(pq);
		return NULL;
	}

	// asignar valores
	pq->arr = arr;
	pq->cap = INITIAL_CAP;
	pq->size = 0;
	return pq;
}

/*
Agrega un valor a la cola con la prioridad dada

retorna TRUE si tuvo exito, FALSE si no
*/
BOOLEAN pq_add(PQ pq, void* valor, int prioridad, int es_arbol) {
	// validar argumentos
	if (pq == NULL) return FALSE;
	if (valor == NULL) return FALSE;
	if (prioridad < 0) return FALSE; // se puede usar prioridad negativa???

	// verificar si pq esta lleno, si es asi, agrandar
	if (pq->size == pq->cap) { return FALSE; }

	// crear estructura PrioValue y verificar creación
	PrioValue pv = (PrioValue)malloc(sizeof(struct _PrioValue));
	if (pv == NULL) return FALSE;
	// asignarle los valores
	pv->value = valor;
	pv->prio = prioridad;
	pv->es_arbol = es_arbol;

	// poner en el índice size + 1 y aumentar el size; luego propagar arriba
	int newIndex = pq->size + 1;
	pq->arr[newIndex] = pv;
	pq->size = newIndex;
	_percolate_up(pq, newIndex);

	//printf("Se agrego valor %c con prioridad %d\n", *(char*)pv->value, prioridad);
	return TRUE;
}

/* 
  Saca el valor de menor prioridad (cima del monticulo) y lo guarda en la posicion retVal (paso por referencia)
  retorna FALSE si tiene un error
  retorna TRUE si tuvo EXITO
*/
BOOLEAN pq_remove(PQ pq, void** retVal) {
	if (pq == NULL || retVal == NULL) return FALSE;

	// Guardar el nodo que será eliminado
	PrioValue toRemove = pq->arr[1];
	// Devolver el puntero al nodo
	*retVal = toRemove;

	// poner el ultimo elemento en la cima
	pq->arr[1] = pq->arr[pq->size];
	// decrementar el tamaño
	pq->size--;
	// propagar hacia abajo
	_percolate_down(pq, 1);
	return TRUE;
}

/* retorna el tamaño de la cola de prioridad, 
   retorna 0 si hubo error 
 */
int pq_size(PQ pq) {
	if (pq == NULL) return 0;
	return pq->size;
}

/* Destruye la cola de prioridad, 
retorna TRUE si tuvo exito
retorna FALSE si tuvo error*/
BOOLEAN pq_destroy(PQ pq) {
	if (pq == NULL) return FALSE;

	// liberar los nodos
	for (int i = 1; i <= pq->size; i++) {
		free(pq->arr[i]);
	}
	// liberar array y pq
	free(pq->arr);
	free(pq);
	return TRUE;
}

// FUNCIONES ADICIONALES AGREGADAS -----------

/* Realiza la operacion de propagar arriba*/
void _percolate_up(PQ pq, int i) {
	if (pq == NULL || i <= 1) return; // caso base

	// calcular el padre
	int pa = i / 2;

	// si el padre tiene mayor prioridad que el nuevo, intercambiar y propagar arriba recursivamente
	if (pq->arr[pa]->prio > pq->arr[i]->prio) {
		PrioValue temp = pq->arr[pa];
		pq->arr[pa] = pq->arr[i];
		pq->arr[i] = temp;
		_percolate_up(pq, pa);
	}
}

/* Realiza la operacion de propagar abajo
basado en el pseudocodigo de heapifyDown de UW Computer Science
https://pages.cs.wisc.edu/~mcw/cs367/lectures/heaps.html
*/
void _percolate_down(PQ pq, int i) {
	if (pq == NULL || i >= pq->size) return; // caso base

	// calcular hijos 
	int left = i * 2;
	int right = i * 2 + 1;
	int lowest = i; // iniciar asumiendo que el padre es el menor

	// si el hijo izq existe y es menor al padre, es lowest actual 
	if (left <= pq->size && pq->arr[left]->prio < pq->arr[lowest]->prio) {
		lowest = left;
	}
	// luego, verificar si el derecho existe y si es menor a lowest actual
	if (right <= pq->size && pq->arr[right]->prio < pq->arr[lowest]->prio) {
		lowest = right;
	}

	// si lowest es diferente al padre, intercambiamos y propagamos abajo
	if (lowest != i) {
		PrioValue temp = pq->arr[i];
		pq->arr[i] = pq->arr[lowest];
		pq->arr[lowest] = temp;
		_percolate_down(pq, lowest);
	}

}

/* Imprime un PQ de chars con el formato valor = prioridad */
void print_pq(PQ pq) {
	if (pq == NULL) return;
	printf("\n");
	// recorrer la pq e imprimir cada valor
	for (int i = 1; i <= pq->size; i++) {
		// acceder al valor del puntero value como un char
		char ch = *(char*)(pq->arr[i]->value);
		if (ch == ' ') {
			printf("\n espacio = %d", pq->arr[i]->prio);
		}
		else if (ch == '\n') {
			printf("\n newline = %d", pq->arr[i]->prio);
		}
		else {
			printf("\n%c = %d", ch, pq->arr[i]->prio);
		}
	}
}
