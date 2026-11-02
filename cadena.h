#ifndef CADENA_H_INCLUDED
#define CADENA_H_INCLUDED

#include <stddef.h>

#define SEPARADOR_CSV ';'
#define AMAYUS(X) \
    ((X) >= 'a' && (X) <= 'z' ? (X) - ('a' - 'A') : (X))


void arreglar_apyn(char *);      /* normaliza "Apellidos, Nombres" */
void arreglar_titulo(char *);    /* normaliza titulos (sin coma) */
void quitar_salto(char *);       /* elimina '\n' y '\r' finales */
int  separar_campos(char *linea, char *campos[], int max);
int  es_numero(const char *);    /* solo digitos (opcionalmente '-') */
int  cadena_vacia(const char *);
void copiar_cadena(char *dest, const char *orig, size_t tam);
void reemplazar_caracter(char *cadena, char a , char b);

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

void arreglar_apyn_ALU(char *s);
int separar_campos_ALU(char *linea, char *campos[], int max);
void cadena_a_mayusculas_ALU(char *cadena);

#endif /* CADENA_H_INCLUDED */
