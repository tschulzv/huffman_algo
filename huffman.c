#define _CRT_SECURE_NO_WARNINGS
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
static void imprimirNodoReconstruido(Arbol nodo);
static int _es_hoja(Arbol nodo);
static void _escribir_arbol(Arbol a, BitStream out);
static Arbol _crear_nodo_interno(Arbol izq, Arbol der);
static void _recorrer(Arbol a, BitStream in, BitStream out);

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
       nota: le agregué {0} para inicializar las frceuencias a 0
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
    
    // LEER Y RECONSTRUIR EL ARBOL -------------
    /* Leer Arbol de Huffman */
    arbol = leer_arbol(in);
    arbol_imprimir(arbol, imprimirNodoReconstruido);

    // LEER EL TEXTO COMPRIMIDO  Y DESCOMPRIMIR
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
    if (pq == NULL) { return NULL; }

    // recorrer array de ascii y agregar a pq los caracteres con frecuencia > 0
    for (int i = 0; i < 256; i++) {
        if (frecuencias[i] > 0) {
            char* ch = malloc(sizeof(char));
            if (ch == NULL) { return NULL; }
            *ch = (char)i;
            pq_add(pq, ch, frecuencias[i], 0);
            printf("AGREGAR A PQ: '%c' (ASCII %d), FRECUENCIA: %d\n", *ch, i, frecuencias[i]);

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
        /*
        / si pv1 aun no es un arbol, convertir su valor a char
        if (pv1->es_arbol != 0) {
            char1 = (char*)pv1->value;
            izq = arbol_crear(char1);
        }
        else {
            izq = (Arbol)(pv1->value);
        }
        if (pv2->es_arbol != 0) {
            char2 = (char*)pv2->value;
            der = arbol_crear(char1);
        }
        else {
            der = (Arbol)(pv2->value);
        }

        
        char* char1 = (char*)malloc(sizeof(char));
        char* char2 = (char*)malloc(sizeof(char));
        Arbol izq;
        Arbol der;

        */

        int suma = pv1->prio + pv2->prio;
        printf("\nSuma de prioridades es:% d", suma);
        // puntero a un int para el valor del arbol
        int* pSuma = malloc(sizeof(int));
        if (pSuma == NULL) return NULL;
        *pSuma = suma;

        // crear el nuevo arbol con la suma como valor de raiz, y los caracteres/arboles de pv1 y pv2 como hijos
        Arbol arbol = arbol_crear(pSuma);
        printf("se creo el arbol con valor: %d", *pSuma);

        // si el valor de pv1 o pv2 ya es un arbol, no se necesita crear un nuevo nodo; si aun es un char se crea el nodo
        Arbol izq = pv1->es_arbol == 1 ? (Arbol)(pv1->value) : arbol_crear(pv1->value);
        Arbol der = pv2->es_arbol == 1 ? (Arbol)(pv2->value) : arbol_crear(pv2->value);

        if (arbol == NULL || izq == NULL || der == NULL) return NULL;
        
        // agregar hijos al arbol 
        arbol_agregarIzq(arbol, izq);
        arbol_agregarDer(arbol, der);
     
        // meter el arbol de nuevo en pq
        pq_add(pq, arbol, suma, 1);
    }
    // al final, queda un solo elemento en pq, que es el arbol
    PrioValue pv = NULL;
    pq_remove(pq, (void**)&pv);
    printf("ya se termino el arbol");

    if (pv == NULL) { return 0; }
    printf("\nprioridad: %d", pv->prio);
    Arbol a = (Arbol)pv->value;
    int *numero = (int*)arbol_valor(a);
    
    printf("se obtuvo un arbol con valor en la raiz: %d", *numero); // error aqui
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

    // crear un campobits que usaremos para guardar los codigos mientras recorremos el árbol
    campobits* bits = (campobits*)malloc(sizeof(struct _campobits));
    CONFIRM_NOTNULL(bits, 1);
    bits->bits = 0;
    bits->tamano = 0;

    // recorrer el arbol, poniendo el 'codigo' de cada caracter en la tabla
    crear_tabla(tabla, T, bits);

    // creacion del archivo bitstream
    int i = 0;
    int j = 0;

    out = OpenBitStream(salida, "w");

    // ESCRITURA DEL ARBOL -----------------------------------------
    // escribimos en preorden el arbol en el archivo de salida
    arbol_preorden(T, _escribir_arbol, out);


    // COMPRESION DEL TEXTO  ---------------------------------------
    // abirir el archivo de entrada
    in = fopen(entrada, "r");
    if (in == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    campobits* b = NULL;
    while (1) {
        char c = fgetc(in);
        if (c == EOF) {
            break;
        }
        //b = &tabla[c]; 
        /*
        for (i = 0; i < b->tamano; i++) { //escribir los bits del campobit, es decir el codigo del caracter
            int bit = bits_leer(b, i);
            PutBit(out, bit);
        }
        */

        // buscar el campobits correspondiente en la tabla (indice = ascii del caracter) 
        // recorrer sus bits y poner en el archivo
        for (i = 0; i < tabla[c].tamano; i++) {
            int bit = 0 != (tabla[c].bits & (0x1 << i));
            PutBit(out, bit);
        }
    }

    // liberar memoria utilizada
    free(b);
   

    
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
    if (_es_hoja(T)) {
        // obtener el valor del nodo
        char* c = (char*)arbol_valor(T);
        // guardamos en la tabla de campobits usando el valor en ascii del char como indice
        tabla[*c] = *bits;
        printf("se escribio codigo de %c", *c);
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
    CONFIRM_NOTNULL(bs, NULL);
    // leer un bit de bs
    int bit = GetBit(bs);
    // si el bit leido es una hoja debemos leer el ascii
    if (bit == 1) {
        char c = (char)GetByte(bs);
        // creamos la hoja con el char como valor
        return arbol_crear(c);
    }
    else if (bit == 0){
        // leer recursivamente creando nodo izq y der
        Arbol izq = leer_arbol(bs);
        Arbol der = leer_arbol(bs);
        // creamos el nodo interno que une ambos arboles
        _crear_nodo_interno(izq, der);
    }
    else { // si hay un error de lectura
        return NULL;
    }

}

/* Esto se utiliza como parte de la descompresion (ver descomprimir())..
   
   Ahora lee todos los bits que quedan en in, y escribelos como bytes
   en out. Utiliza los bits para navegar por el arbol de huffman, y
   cuando llegues a una hoja escribe el codigo ASCII al out con PutByte()
   y vuelve a comenzar a procesar bits desde la raiz.
   
   Sigue con este proceso hasta que no hay mas bits en in.
*/   
static void decodificar(BitStream in, BitStream out, Arbol arbol) {
    CONFIRM_RETURN(arbol);
    CONFIRM_RETURN(in);
    CONFIRM_RETURN(out);
    Arbol raiz = (Arbol)arbol;

    // mientras aun quedan bits en in
    while (!IsEmptyBitStream(in)) {
        // recorre hasta encontrar una hoja
        // cuando encuentra la hoja escribe el caracter
        // luego vuelve a recorrer desde la raiz
        _recorrer(raiz, in, out);
    }
   
}
// recorre un arbol y si llega a una hoja pone el caracter en el archivo
static void _recorrer(Arbol a, BitStream in, BitStream out) {
    if (_es_hoja(a)) {
        char c = (char)arbol_valor(a);
        PutByte(out, c); // pone el caracter en el archivo de salida
        return;
    }
    else { 
        // obtener un bit
        int bit = GetBit(in);
        if (bit == 0) { // si es 0, ir a la izquierda
            _recorrer(arbol_izq(a), in, out);
        }
        if (bit == 1) { // si es 1 a la derecha
            _recorrer(arbol_der(a), in, out);
        }
    }
    
}


/* Esto es para imprimir nodos..
   Tal vez tengas mas de uno de estas funciones debendiendo
   de como decidiste representar los valores del arbol durante
   la compresion y descompresion.
*/
static void imprimirNodo(Arbol nodo) {
    CONFIRM_RETURN(nodo);
    // Si es una hoja (sin hijos)
    if (arbol_izq(nodo) == NULL && arbol_der(nodo) == NULL) {
        char* c = (char*)arbol_valor(nodo);
        printf("'%c'", *c);  // Muestra el carácter
    }
    else { // si es un nodo interno 
        int* freq = (int*)arbol_valor(nodo);
        printf("%d", *freq);  // Muestra la suma de frecuencias
    }
}

// imprimir nodo del arbol reconstruido
// en este caso, ya no tenemos valores en os nodos internos, asi que imprimimos un * 
static void imprimirNodoReconstruido(Arbol nodo) {
    CONFIRM_RETURN(nodo);
    // Si es una hoja (sin hijos)
    if (arbol_izq(nodo) == NULL && arbol_der(nodo) == NULL) {
        char c = (char)arbol_valor(nodo);
        printf("'%c'", c);  // Muestra el carácter
    }
    else { // si es un nodo interno mostrar *
        printf("*");  
    }
}

static int _es_hoja(Arbol nodo) {
    CONFIRM_NOTNULL(nodo, -1);
    return (arbol_izq(nodo) == NULL && arbol_der(nodo) == NULL);
}

static void _escribir_arbol(Arbol a, BitStream out) {
    CONFIRM_RETURN(a);
    CONFIRM_RETURN(out);
    // si el nodo es una hoja ponemos un 1 y el byte del caracter
    if (_es_hoja(a)) {
        PutBit(out, 1);
        char* c = (char*)arbol_valor(a);
        PutByte(out, *c);
    }
    else { // si no ponemos un 0 
        PutBit(out, 0);
    }
}

// funcion usada para reconstruir el arbol
// crea un nodo sin valor, con hijo izq y derecho
static Arbol _crear_nodo_interno(Arbol izq, Arbol der) {
    CONFIRM_NOTNULL(izq, NULL);
    CONFIRM_NOTNULL(der, NULL);
    Arbol a = arbol_crear(NULL);
    arbol_agregarIzq(a, izq);
    arbol_agregarDer(a, der);
}

/*
Funcion utilizada para verificar si el PrioValue sacado de la PQ es solo un caracter, o si ya contiene un arbol
En este caso, un arbol dentro de la cola ya deberia tener un hijo izq y un hijo derecho, por lo que se verifica que no sean NULL

static int _es_subarbol(void* valor) {
    Arbol nodo = (Arbol)valor;
    if 
    return 0;
    //return nodo != NULL && (arbol_izq(nodo)!= NULL || arbol_der(nodo) != NULL);
} */