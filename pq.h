#ifndef DEFINE_PQ_H
#define DEFINE_PQ_H

/*Definicion del API de la cola de prioridades, la implementacion va en pq.c*/

/* macros de definicion de tipos de datos boolean */
#define TRUE 1
#define FALSE 0
#define BOOLEAN int

/* Implementacion de una cola de prioridades usando un Monticulo (Heap) */

/* PrioValue es un contenedor para almacenar la combinacion de Prioridad+Valor dentro del arreglo*/
// agregue un campo int es_arbol para facilitar el manejo de chars vs arboles en la pq
typedef struct _PrioValue {
	int prio;
	void* value;
	int es_arbol;
}*PrioValue;

/*Heap es la estructura que contiene el arreglo (de PrioValues), la capacidad del arreglo y el tamano del monticulo */
typedef struct Heap {
	PrioValue* arr;
	int cap;
	int size;
}*PQ;


/* Crea la cola de prioridad e inicializa sus atributos
retorna un puntero a la cola de prioridad
retorna NULL si hubo error*/
PQ pq_create();


/*
  Agrega un valor a la cola con la prioridad dada
  
  retorna TRUE si tuvo exito, FALSE si no
*/
BOOLEAN pq_add(PQ pq, void* valor, int prioridad, int es_arbol);

/*
Saca el valor de menor prioridad (cima del monticulo) y lo guarda en la posicion retVal (paso por referencia)
retorna FALSE si tiene un error
retorna TRUE si tuvo EXITO
*/
BOOLEAN pq_remove(PQ pq, void** retVal);

/* retorna el tamaño de la cola de prioridad,
retorna 0 si hubo error
*/
int pq_size(PQ pq);

/* Destruye la cola de prioridad,
retorna TRUE si tuvo exito
retorna FALSE si tuvo error*/
BOOLEAN pq_destroy(PQ pq);

// funciones adicionales

/* Realiza la operación de propagar arriba*/
void _percolate_up(PQ pq, int i);

/* Realiza la operación de propagar abajo*/
void _percolate_down(PQ pq, int i);


#endif

