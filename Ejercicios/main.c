#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_ESTADOS 500

typedef struct Estado Estado;
typedef struct Transicion Transicion;

//Profe todos los words se generan dentro de la carpeta donde es mi programa. muchas gracias


struct Transicion {
    char simbolo;
    Estado *destino;
    Transicion *siguiente;
};

struct Estado {
    int id;
    int esFinal;
    Transicion *transiciones;
};

Estado* crearEstado(int id, int esFinal) {
    Estado *e = (Estado*)malloc(sizeof(Estado));
    e->id = id;
    e->esFinal = esFinal;
    e->transiciones = NULL;
    return e;
}

void agregarTransicion(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion*)malloc(sizeof(Transicion));
    t->simbolo = simbolo;
    t->destino = destino;
    t->siguiente = origen->transiciones;
    origen->transiciones = t;
}

int simularAFN(Estado *estado, const char *cadena, int pos) {
    if (pos == strlen(cadena)) {
        if (estado->esFinal) return 1;
        for (Transicion *t = estado->transiciones; t; t = t->siguiente) {
            if (t->simbolo == 'E' && simularAFN(t->destino, cadena, pos))
                return 1;
        }
        return 0;
    }

    char c = cadena[pos];
    for (Transicion *t = estado->transiciones; t; t = t->siguiente) {
        if (t->simbolo == c && simularAFN(t->destino, cadena, pos+1))
            return 1;
        if (t->simbolo == 'E' && simularAFN(t->destino, cadena, pos))
            return 1;
    }
    return 0;
}

typedef struct NodoArbol {
    char tipo;
    char valor;
    struct NodoArbol *izq;
    struct NodoArbol *der;
} NodoArbol;

typedef struct {
    Estado *inicio;
    Estado *fin;
} FragmentoAFN;

int contadorEstados = 0;

NodoArbol* crearNodo(char tipo, char valor, NodoArbol *izq, NodoArbol *der) {
    NodoArbol *nodo = (NodoArbol*)malloc(sizeof(NodoArbol));
    nodo->tipo = tipo;
    nodo->valor = valor;
    nodo->izq = izq;
    nodo->der = der;
    return nodo;
}

// ============================================================================
// IMPRESOR JERÁRQUICO
// ============================================================================
int calcularAltura(NodoArbol *raiz) {
    if (!raiz) return 0;
    int altIzq = calcularAltura(raiz->izq);
    int altDer = calcularAltura(raiz->der);
    return (altIzq > altDer ? altIzq : altDer) + 1;
}

void imprimirNivelRepresentacion(NodoArbol *raiz, int nivel, int distancia) {
    if (nivel == 1) {
        for (int i = 0; i < distancia; i++) printf(" ");
        if (raiz) {
            printf("%c", (raiz->tipo == 'o' ? raiz->valor : raiz->tipo));
        } else {
            printf(" ");
        }
        for (int i = 0; i < distancia + 1; i++) printf(" ");
    } else if (nivel > 1) {
        imprimirNivelRepresentacion(raiz ? raiz->izq : NULL, nivel - 1, distancia);
        imprimirNivelRepresentacion(raiz ? raiz->der : NULL, nivel - 1, distancia);
    }
}

void mostrarArbolPiramidal(NodoArbol *raiz) {
    int altura = calcularAltura(raiz);
    for (int i = 1; i <= altura; i++) {
        int espaciado = (1 << (altura - i)) * 3;
        imprimirNivelRepresentacion(raiz, i, espaciado);
        printf("\n\n");
    }
}

FragmentoAFN construirAFN(NodoArbol *raiz) {
    FragmentoAFN automataActual = {NULL, NULL};
    if (!raiz) return automataActual;

    if (raiz->tipo == 'o') {
        Estado *inicio = crearEstado(contadorEstados++, 0);
        Estado *fin = crearEstado(contadorEstados++, 0);
        agregarTransicion(inicio, raiz->valor, fin);
        automataActual.inicio = inicio; automataActual.fin = fin;
    }
    else if (raiz->tipo == '.') {
        FragmentoAFN izqFrag = construirAFN(raiz->izq);
        FragmentoAFN derFrag = construirAFN(raiz->der);
        agregarTransicion(izqFrag.fin, 'E', derFrag.inicio);
        automataActual.inicio = izqFrag.inicio; automataActual.fin = derFrag.fin;
    }
    else if (raiz->tipo == '|') {
        FragmentoAFN izqFrag = construirAFN(raiz->izq);
        FragmentoAFN derFrag = construirAFN(raiz->der);
        Estado *inicio = crearEstado(contadorEstados++, 0);
        Estado *fin = crearEstado(contadorEstados++, 0);
        agregarTransicion(inicio, 'E', izqFrag.inicio);
        agregarTransicion(inicio, 'E', derFrag.inicio);
        agregarTransicion(izqFrag.fin, 'E', fin);
        agregarTransicion(derFrag.fin, 'E', fin);
        automataActual.inicio = inicio; automataActual.fin = fin;
    }
    else if (raiz->tipo == '*') {
        FragmentoAFN izqFrag = construirAFN(raiz->izq);
        Estado *inicio = crearEstado(contadorEstados++, 0);
        Estado *fin = crearEstado(contadorEstados++, 0);
        agregarTransicion(inicio, 'E', izqFrag.inicio);
        agregarTransicion(inicio, 'E', fin);
        agregarTransicion(izqFrag.fin, 'E', izqFrag.inicio);
        agregarTransicion(izqFrag.fin, 'E', fin);
        automataActual.inicio = inicio; automataActual.fin = fin;
    }
    return automataActual;
}

void liberarArbol(NodoArbol *raiz) {
    if (!raiz) return;
    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);
    free(raiz);
}

//.DOT (ÁRBOL)
void generarDotRec(NodoArbol *raiz, FILE *f, int *contador) {
    if (raiz == NULL)
        return;

    int idActual = (*contador)++;
    char caracterAMostrar = (raiz->tipo == 'o') ? raiz->valor : raiz->tipo;

    fprintf(f, "    N%d[label=\"%c\"];\n", idActual, caracterAMostrar);

    if (raiz->izq != NULL) {
        int idHijoIzq = *contador;
        generarDotRec(raiz->izq, f, contador);
        fprintf(f, "    N%d -> N%d;\n", idActual, idHijoIzq);
    }

    if (raiz->der != NULL) {
        int idHijoDer = *contador;
        generarDotRec(raiz->der, f, contador);
        fprintf(f, "    N%d -> N%d;\n", idActual, idHijoDer);
    }
}

void generarDot(NodoArbol *raiz, const char *nombreArchivo) {
    FILE *f = fopen(nombreArchivo, "w");
    if (f == NULL) {
        printf("No se pudo crear el archivo DOT\n");
        return;
    }

    fprintf(f, "digraph {\n");
    int contador = 0;
    generarDotRec(raiz, f, &contador);
    fprintf(f, "}\n");
    fclose(f);
    printf("Su archivo '%s' fue generado con exito\n", nombreArchivo);
}

//Formato .DOT (AFN)
void generarDotAFNRec(Estado *actual, FILE *f, int *visitado) {
    if (actual == NULL || visitado[actual->id]) return;

    visitado[actual->id] = 1;

    if (actual->esFinal) {
        fprintf(f, "    q%d [shape=doublecircle];\n", actual->id);
    } else {
        fprintf(f, "    q%d [shape=circle];\n", actual->id);}
    for (Transicion *t = actual->transiciones; t != NULL; t = t->siguiente) {
        fprintf(f, "    q%d -> q%d [label=\"%c\"];\n", actual->id, t->destino->id, t->simbolo);
        generarDotAFNRec(t->destino, f, visitado);
    }
}

void generarDotAFN(FragmentoAFN afn, const char *nombreArchivo) {
    FILE *f = fopen(nombreArchivo, "w");
    if (f == NULL) {
        printf("No se pudo crear el archivo del AFN.\n");
        return;
    }

    fprintf(f, "digraph AFN {\n");
    fprintf(f, "    rankdir=LR;\n");
    fprintf(f, "    inicio [shape=point];\n");
    fprintf(f, "    inicio -> q%d;\n", afn.inicio->id);

    int visitado[MAX_ESTADOS] = {0};
    generarDotAFNRec(afn.inicio, f, visitado);

    fprintf(f, "}\n");
    fclose(f);
    printf("Su archivo AFN '%s' fue generado con exito\n", nombreArchivo);
}

FragmentoAFN CrearAFN(char simbolo) {
    FragmentoAFN automataSencillo;
    Estado *inicio = crearEstado(contadorEstados++, 0);
    Estado *fin = crearEstado(contadorEstados++, 0);

    agregarTransicion(inicio, simbolo, fin);

    automataSencillo.inicio = inicio;
    automataSencillo.fin = fin;
    return automataSencillo;
}

//Unión
FragmentoAFN unionAFN(FragmentoAFN A, FragmentoAFN B) {
    FragmentoAFN resultadoUnion;
    Estado *nuevoInicio = crearEstado(contadorEstados++, 0);
    Estado *nuevoFin = crearEstado(contadorEstados++, 0);

    A.fin->esFinal = 0;
    B.fin->esFinal = 0;

    agregarTransicion(nuevoInicio, 'E', A.inicio);
    agregarTransicion(nuevoInicio, 'E', B.inicio);
    agregarTransicion(A.fin, 'E', nuevoFin);
    agregarTransicion(B.fin, 'E', nuevoFin);

    resultadoUnion.inicio = nuevoInicio;
    resultadoUnion.fin = nuevoFin;
    return resultadoUnion;
}

// Concatenación
FragmentoAFN concatenacionAFN(FragmentoAFN A, FragmentoAFN B) {
    FragmentoAFN resultadoConcat;
    A.fin->esFinal = 0;

    agregarTransicion(A.fin, 'E', B.inicio);

    resultadoConcat.inicio = A.inicio;
    resultadoConcat.fin = B.fin;
    return resultadoConcat;
}

 //Clausura de Kleene
FragmentoAFN clausuraAFN(FragmentoAFN A) {
    FragmentoAFN resultadoClausura;
    Estado *nuevoInicio = crearEstado(contadorEstados++, 0);
    Estado *nuevoFin = crearEstado(contadorEstados++, 0);

    A.fin->esFinal = 0;

    agregarTransicion(nuevoInicio, 'E', A.inicio);
    agregarTransicion(nuevoInicio, 'E', nuevoFin);
    agregarTransicion(A.fin, 'E', A.inicio);
    agregarTransicion(A.fin, 'E', nuevoFin);

    resultadoClausura.inicio = nuevoInicio;
    resultadoClausura.fin = nuevoFin;
    return resultadoClausura;
}

FragmentoAFN procesarArbolToAFN(NodoArbol *raiz) {
    FragmentoAFN resultadoAFN = construirAFN(raiz);
    if (resultadoAFN.fin != NULL) {
        resultadoAFN.fin->esFinal = 1;
    }
    return resultadoAFN;
}

void limpiarPantalla(){
#ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    int opcion = 0;
    NodoArbol *nodoA = crearNodo('o', 'a', NULL, NULL);
    NodoArbol *nodoB = crearNodo('o', 'b', NULL, NULL);
    NodoArbol *nodoC = crearNodo('o', 'c', NULL, NULL);

    NodoArbol *subClausuraA = crearNodo('*', '\0', nodoA, NULL);
    NodoArbol *subConcatenacionBC = crearNodo('.', '\0', nodoB, nodoC);
    NodoArbol *raizER = crearNodo('|', '\0', subClausuraA, subConcatenacionBC);

    FragmentoAFN afnResultante = {NULL, NULL};

    do {
        printf("\n-- Menu de programas --\n");
        printf("1) Implementar arbol para representar una expresion regular\n");
        printf("2) Dar la salida del arbol formato .DOT \n");
        printf("3) Generar archivos de los incisos (b), (c) y (d) en formato .DOT \n");
        printf("4) Genrar el AFN de la entidad regular\n");
        printf("0) Salir\n");
        printf("Seleccione una opcion\n");

        scanf("%d", &opcion);

        switch(opcion) {
            case 1: {
                printf(" REPRESENTACION DEL ARBOL BINARIO DE LA ER:\n");
                printf("                a* | (b . c) \n\n");
                mostrarArbolPiramidal(raizER);

                afnResultante = construirAFN(raizER);
                afnResultante.fin->esFinal = 1;
                printf("AFN generado exitosamente a partir del arbol.\n");
                contadorEstados = 0;

                //limpar pantalla
                printf("\nPresione Enter para continuar...");
                getchar();
                getchar();
                limpiarPantalla();
                break;
            }
            case 2:
                printf("\nGenerando archivo.dot para el Arbol...\n");
                generarDot(raizER, "expresion.dot");

                //limpiar pantalla
                printf("\nPresione Enter para continuar...");
                getchar();
                getchar();
                limpiarPantalla();
                break;

            case 3: {
                printf("\n Creando archivos .DOT en simultaneo...\n");


                contadorEstados = 0;
                FragmentoAFN AFN_A_union = CrearAFN('a');
                FragmentoAFN AFN_B_union = CrearAFN('b');
                FragmentoAFN resultadoUnion = unionAFN(AFN_A_union, AFN_B_union);
                resultadoUnion.fin->esFinal = 1;
                generarDotAFN(resultadoUnion, "UnionAyB.dot");


                contadorEstados = 0;
                FragmentoAFN AFN_A_concat = CrearAFN('a');
                FragmentoAFN AFN_B_concat = CrearAFN('b');
                FragmentoAFN resultadoConcat = concatenacionAFN(AFN_A_concat, AFN_B_concat);
                resultadoConcat.fin->esFinal = 1;
                generarDotAFN(resultadoConcat, "ConcatenacionAyB.dot");


                contadorEstados = 0;
                FragmentoAFN AFN_A_clausura = CrearAFN('a');
                FragmentoAFN resultadoClausura = clausuraAFN(AFN_A_clausura);
                resultadoClausura.fin->esFinal = 1;
                generarDotAFN(resultadoClausura, "clausuraA.dot");

                //limpiar pantalla
                printf("\nPresione Enter para continuar...");
                getchar();
                getchar();
                limpiarPantalla();

                break;
            }
                case 4: {
                printf(" Pasamos de ER a AFN mediante el algortimo de THOMPSON):\n");
                printf(" Expresion utilizada: a* | (b . c)\n");
                FragmentoAFN afnEquivalente = procesarArbolToAFN(raizER);
                generarDotAFN(afnEquivalente, "AFN_Thompson.dot");
                contadorEstados = 0;

                //Limpiar pantalla
                printf("\nPresione Enter para continuar...");
                getchar();
                getchar();
                limpiarPantalla();
                break;
            }
            case 0:
                printf("\nSaliendo del programa...\n");
                break;
            default:
                printf("\nOpcion invalida.\nElija una opcion valida");
                break;
        }
    } while(opcion != 0);
    liberarArbol(raizER);
    return 0;
}
