
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arbol.h"
#include "pq.h"
#include "bitstream.h"
#include "confirm.h"

/*====================================================
     Constantes
  ====================================================*/

#define NUM_CHARS 256

/*
estructura para almacenar valores de un nodo de un arbol, 
c es el caracter
frec es la frecuencia
*/
typedef struct _keyvaluepair {
    char c;
    int frec;

} keyvaluepair;

/*====================================================
     Campo de bits.. agrega funciones si quieres
     para facilitar el procesamiento de bits.
  ====================================================*/

typedef struct _campobits {
    unsigned int bits;
    int tamano;
} campobits;

/*
 estructura para relacionar caracteres
 con sus correspondientes codigos (campobits)
*/
typedef struct _codigochar {
    char c;
    campobits codigo;
};



/* Esto utiliza aritmetica de bits para agregar un
   bit a un campo.
   
   Supongamos que bits->bits inicialmene es (en binario):
   
      000110001000
      
   y le quiero agregar un 1 en vez del segundo 0 (desde izq).
   Entonces, creo una "mascara" de la siguiente forma:
   
      1 << 11   me da 0100000000000

   Y entonces si juntamos los dos (utilizando OR binario):      
      000110001000
    | 0100000000000
    ----------------
      010110001000

    Esta funcion utiliza bits->tamano para decidir donde colocar
    el siguiente bit.
    
    Nota: asume que bits->bits esta inicialmente inicializado a 0,
    entonces agregar un 0, no requiere mas que incrementar bits->tamano.
*/
      
static void bits_agregar(campobits* bits, int bit) {
    CONFIRM_RETURN(bits);
    CONFIRM_RETURN((unsigned int)bits->tamano < 8*sizeof(bits->bits));
    bits->tamano++;
    if (bit) {
        bits->bits = bits->bits | ( 0x1 << (bits->tamano-1));
    } 
}
/*
    funcion de utilidad para leer un bit dentro de campobits dado el indice pos
    pos = 0, primer bit 
    pos = 1, segundo bit
    pos = 2, tercer bit
    pos = k, k bit

*/
static int bits_leer(campobits* bits, int pos) {
    CONFIRM_TRUE(bits,0);
    CONFIRM_TRUE(!(pos < 0 || pos > bits->tamano),0);
    // para saber si campobits tiene un 1 o 0 en la posicion dada 
    // recorro bits usando shift << y >>
    int bit = (bits->bits & (0x1 << (pos))) >> (pos);
    return bit;
}

static void testCampobitsBitstream() {
    BitStream bs = NULL;
    BitStream bsIn = NULL;
    char* testbitstreamtxt = "testbitsteam.txt";
    int i = 0;
    // crear un campobits
    campobits* b = (campobits*)malloc(sizeof(struct _campobits));
    CONFIRM_RETURN(b);
    b->bits = 0;
    b->tamano = 0;
    // ej quiero codificar 00101 
    bits_agregar(b, 0);
    bits_agregar(b, 0);
    bits_agregar(b, 1);
    bits_agregar(b, 0);
    bits_agregar(b, 1);
    // crear un archivo y escribir bit a bit
    bs = OpenBitStream(testbitstreamtxt, "w");
    // para escribir en un archivo PutBit agrega bits 
    // ej recorro el campobits y agrego bit a bit
    for (i = 0; i < b->tamano; i++) {
        int bit = bits_leer(b, i);
        PutBit(bs, bit);
    }
    // PutByte para escribir 1 byte completo, 
    // ej agrego un caracter
    PutByte(bs, 'z');
    // no olvidar cerrar el archvio
    CloseBitStream(bs);
    // y liberar memoria utilizada
    free(b);
    // Mi archivo entonces contiene: 00101z
    // si quiero leer el mismo
    bsIn = OpenBitStream(testbitstreamtxt, "r");
    // leer bit a bit
    printf("%d", GetBit(bsIn));
    printf("%d", GetBit(bsIn));
    printf("%d", GetBit(bsIn));
    printf("%d", GetBit(bsIn));
    printf("%d", GetBit(bsIn));
    // leer un byte
    printf("%c\n", GetByte(bsIn));
    CloseBitStream(bsIn);



}
static void imprimirNodoEjemplo(Arbol nodo) {
    CONFIRM_RETURN(nodo);
    keyvaluepair* val = (keyvaluepair*)arbol_valor(nodo);
    printf("%d,%c\n", val->frec, val->c);
}
static void testArbol() {

    /* si quiero crear un arbol que contiene:
    char freq
    a    4
    s    2
    entonces tengo un nodo padre
    con freq 6 y dos hijos, con freq 2  y 4 con sus caracteres correspondientes
    */
    keyvaluepair* v1 = malloc(sizeof(struct _keyvaluepair));
    keyvaluepair* v2 = malloc(sizeof(struct _keyvaluepair));
    keyvaluepair* v3 = malloc(sizeof(struct _keyvaluepair));
    Arbol n1;
    Arbol n2;
    Arbol n3;
    CONFIRM_RETURN(v1);
    CONFIRM_RETURN(v2);
    CONFIRM_RETURN(v3);
    v1->c = 'a';
    v1->frec = 4;
    v2->c = 's';
    v2->frec = 2;
    v3->c = ' ';
    v3->frec = 6;
    n1 = arbol_crear(v1);
    n2 = arbol_crear(v2);
    n3 = arbol_crear(v3);
    arbol_agregarIzq(n3, n2);
    arbol_agregarDer(n3, n1);
    //ejemplo de recorrer el arbol e imprimir nodos con valor keyvaluepair
    arbol_imprimir(n3, imprimirNodoEjemplo);
    arbol_destruir(n3);
    free(v1);
    free(v2);
    free(v3);
}

void campobitsDemo() {
    printf("***************CAMPOBITS DEMO*******************\n");
    testCampobitsBitstream();
    printf("***************ARBOL DEMO******************\n");
    testArbol();
    printf("***************FIN DEMO*****************\n");
}
/*====================================================
     Declaraciones de funciones 
  ====================================================*/

/* Puedes cambiar esto si quieres.. pero entiende bien lo que haces */
static int calcular_frecuencias(int* frecuencias, char* entrada);
static Arbol crear_huffman(int* frecuencias);
static int codificar(Arbol T, char* entrada, char* salida);
static void crear_tabla(campobits* tabla, Arbol T, campobits *bits);


static Arbol leer_arbol(BitStream bs);
static void decodificar(BitStream in, BitStream out, Arbol arbol);

static void imprimirNodo(Arbol nodo);

/*====================================================
     Implementacion de funciones publicas
  ====================================================*/

/*
  Comprime archivo entrada y lo escriba a archivo salida.
  
  Retorna 0 si no hay errores.
*/
int comprimir(char* entrada, char* salida) {
    
    /* 256 es el numero de caracteres ASCII.
       Asi podemos utilizar un unsigned char como indice.
       nota: le agregu� {0} para inicializar las frceuencias a 0
     */
    int frecuencias[NUM_CHARS] = {0};
    Arbol arbol = NULL;
    /* Primer recorrido - calcular frecuencias */
    CONFIRM_TRUE(0 == calcular_frecuencias(frecuencias, entrada), 0);
            
    arbol = crear_huffman(frecuencias);
    arbol_imprimir(arbol, imprimirNodo); 

    /* Segundo recorrido - Codificar archivo */
    CONFIRM_TRUE(0 == codificar(arbol, entrada, salida), 0);
    
    arbol_destruir(arbol);
    
    return 0;
}


/*
  Descomprime archivo entrada y lo escriba a archivo salida.
  
  Retorna 0 si no hay errores.
*/
int descomprimir(char* entrada, char* salida) {

    BitStream in = 0;
    BitStream out = 0;
    Arbol arbol = NULL;
        
    /* Abrir archivo de entrada */
    in = OpenBitStream(entrada, "r");
    
    /* Leer Arbol de Huffman */
    arbol = leer_arbol(in);
    arbol_imprimir(arbol, imprimirNodo);

    /* Abrir archivo de salida */
    out = OpenBitStream(salida, "w");
    
    /* Decodificar archivo */
    decodificar(in, out, arbol);
    
    CloseBitStream(in);
    CloseBitStream(out);
    return 0;
}

/*====================================================
     Funciones privadas
  ====================================================*/

/* Devuelve 0 si no hay errores */
static int calcular_frecuencias(int* frecuencias, char* entrada) {


    /* Este metodo recorre el archivo contando la frecuencia
       que ocurre cada caracter y guardando el resultado en
       el arreglo frecuencias
    */

    // leer el contenido del archivo caracter por caracter; imprimir error si no se encuentra
    FILE* f = fopen(entrada, "r");
    if (f == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    while (1) {
        char c = fgetc(f);
        if (c == EOF) {
            break;
        }
        // aumentar el conteo del caracter correspondiente en el array de ascii
        // castear a unsigned char para asegurar que los chars sean del 0 a 255
        //frecuencias[(unsigned char)c]++;
        frecuencias[c]++;
    }
    fclose(f);
    
    return 0;
}



/* Crea el arbol huffman en base a las frecuencias dadas */
static Arbol crear_huffman(int* frecuencias) {
    // 1. crear la pq y verificar su creacion correcta
    PQ pq = pq_create();
    if (pq == NULL) { return 1; }

    // recorrer array de ascii y agregar a pq los caracteres con frecuencia > 0
    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            char* ch = malloc(sizeof(char));
            if (ch == NULL) { return 1; }
            *ch = (char)i;
            pq_add(pq, ch, frecuencias[i]);
        }
    }

   /* 2. mientras haya mas de un arbol en la cola, sacar dos arboles y juntarlos       
   reinsertar el nuevo arbol combinado
   */
    while (pq->size > 1) {
        // sacar los dos primeros 
        PrioValue pv1 = NULL;
        PrioValue pv2 = NULL;        
        pq_remove(pq, (void**)&pv1);
        pq_remove(pq, (void**)&pv2);
        if (pv1 == NULL || pv2 == NULL) { return NULL; }
        // suma de sus prioridades
        int suma = pv1->prio + pv2->prio;
      
        // crear estructura PrioValue para la raiz y verificar creacion
        PrioValue raiz = (PrioValue)malloc(sizeof(struct _PrioValue));
        if (raiz == NULL) return NULL;
        // poner como prioridad la suma de prioridades
        raiz->value = NULL;
        raiz->prio = suma;

        // crear el nuevo arbol con la raiz
        Arbol arbol = arbol_crear(raiz);
        if (arbol == NULL) return NULL;
        // poner a pv1 y pv2 como hijos
        arbol_agregarIzq(raiz, pv1);
        arbol_agregarDer(raiz, pv2);

        // meter el arbol de nuevo en pq
        pq_add(pq, arbol, suma);
    }
    // al final, queda un solo elemento en pq, que es el arbol
    PrioValue pvFinal = NULL;
    pq_remove(pq, (void**)&pvFinal);
    Arbol a = (Arbol)pvFinal->value;

    // limpieza
    pq_destroy(pq);
    // retornar el arbol resultante
    return a;
}




static int codificar(Arbol T, char* entrada, char* salida) {
    FILE* in = NULL;
    BitStream out = NULL;
    /* Dado el arbol crear una tabla que contiene la
       secuencia de bits para cada caracter.
       
       Los bits se guardan en compobits que es un struct
       que contiene un int (sin signo) y un tamano.
       La idea es que voy agregando bits al campo de bits
       mientras en un campo (bits), y el numero de bits
       que necesito en otro (tamano)
    */
    // el indice del elemento corresponde a su ascii
    campobits tabla[NUM_CHARS];

    /* Inicializar tabla de campo de bits a cero */
    memset(tabla, 0, NUM_CHARS*sizeof(struct _campobits));

    // crear un campobits que usaremos para guardar los codigos mientras recorremos el �rbol
    campobits* bits = (campobits*)malloc(sizeof(struct _campobits));
    CONFIRM_RETURN(bits);
    bits->bits = 0;
    bits->tamano = 0;

    // recorrer el arbol, poniendo el 'codigo' de cada caracter en la tabla
    crear_tabla(tabla, T, bits);
    
    
    /* Abrir archivos */
    /* TU IMPLEMENTACION VA AQUI .. */

    
    /* Escribir arbol al archivo de salida */

    /* TU IMPLEMENTACION VA AQUI .. 


        Nota: puedes utilizar arbol_preorden() para lograr esto
        facilmente.
        
        El truco es que al escribir el arbol, 
           - Si no es hoja: 
               escribe un bit 0 
           - Si es hoja:
                bit 1 seguido por el byte ASCII que representa el caracter 
        
        Para escribir bits utiliza PutBits() de bitstream.h
        Para escribir bytes utiliza PutByte() de bitstream.h
    */
    


    /* Escribir el texto codificado al archivo*/

    /* 
        TU IMPLEMENTACION VA AQUI .. 
        
        
        Lee todos los datos de nuevo del archivo de entrada
        y agregalos al archivo de salida utilizando la
        tabla de conversion.
        
        Recuerda que tienes que escribir bit por bit utilzando
        PutBit() de bitstream.h. Por ejemplo, dado una secuencia
        de bits podrias escribirlo al archivo asi:
        
            /- Esto escribe todos los bits en un campobits a un
               BitStream  -/
            for (i = 0; i < tabla[c].tamano; i++) {
               int bit =  0 != (tabla[c].bits & (0x1<<i));
               PutBit(out, bit);
            }
        
        Puedes colocarlo en una funcion si quieres

    */

    /* No te olvides de limpiar */
    if (in)
        fclose(in);
    if (out)
        CloseBitStream(out);
       
    return 0;
}

/*
    Recorrer recursivamente el arbol guardando en 'bits' 1 o 0 
    segun avancemos a la derecha o izquierda.
    Cuando encontramos una hoja, es un caracter,
    guardamos en el indice correspondiente a su num en ascii
*/
static void crear_tabla(campobits* tabla, Arbol T, campobits* bits) {
    CONFIRM_RETURN(T);

    // si llegamos una hoja, guardamos el valor de 'bits' en la tabla
    if (arbol_izq(T) != NULL && arbol_der(T) != NULL) {
        // obtener el valor del nodo
        PrioValue pv = (PrioValue)(arbol_valor(T));
        tabla[(unsigned char)pv->value] = *bits;
        return;
    }

    // si no es una hoja seguir recursionando 
    // crear copias desreferenciadas para evitar que se 'mezclen' los valores de distintas llamadas recursivas
    
    // hacia la izquierda, agregando 0
    campobits izquierda = *bits; 
    bits_agregar(&izquierda, 0);
    crear_tabla(tabla, arbol_izq(T), &izquierda);

    // hacia la derecha, agregando 0
    campobits derecha = *bits;
    bits_agregar(&derecha, 1);
    crear_tabla(tabla, arbol_der(T), &derecha);
}

             

/* Esto se utiliza como parte de la descompresion (ver descomprimir())..
   
   Para leer algo que esta guardado en preorden, hay que
   pensarlo un poquito.
   
   Pero basicamente la idea es que vamos a leer el archivo
   en secuencia. Inicialmente, el archivo probablemente va 
   a empezar con el bit 0 representando la raiz del arbol. 
   Luego, tenemos que leer recursivamente (utiliza otra funcion
   para ayudarte si lo necesitas) un nodo izquierdo y uno derecho.
   Si uno (o ambos) son hojas entonces tenemos que leer tambien su
   codigo ASCII. Hacemos esto hasta que todos los nodos tienen sus 
   hijos. (Si esta bien escrito el arbol el algoritmo terminara
   porque no hay mas nodos sin hijos)
*/
static Arbol leer_arbol(BitStream bs) {
  
    /* TU IMPLEMENTACION AQUI */

    return NULL;

}

/* Esto se utiliza como parte de la descompresion (ver descomprimir())..
   
   Ahora lee todos los bits que quedan en in, y escribelos como bytes
   en out. Utiliza los bits para navegar por el arbol de huffman, y
   cuando llegues a una hoja escribe el codigo ASCII al out con PutByte()
   y vuelve a comenzar a procesar bits desde la raiz.
   
   Sigue con este proceso hasta que no hay mas bits en in.
*/   
static void decodificar(BitStream in, BitStream out, Arbol arbol) {
  
    /* TU IMPLEMENTACION AQUI */
}


/* Esto es para imprimir nodos..
   Tal vez tengas mas de uno de estas funciones debendiendo
   de como decidiste representar los valores del arbol durante
   la compresion y descompresion.
*/
static void imprimirNodo(Arbol nodo) {
    /* TU IMPLEMENTACION AQUI */
}
