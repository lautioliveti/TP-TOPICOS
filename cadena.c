#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "cadena.h"

#define ESLETRA(X) ( (((X)>='a' && (X)<='z')||((X)>='A' && (X)<='Z'))? 1 : 0 )
#define ESMAYUS(X) ( ((X) >='A' && (X) <='B') ? 1 : 0 )
#define MAX_STR 100


/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    normaliza un campo Apellidos y Nombres segun el Anexo A.2 (con
                regla de coma).
Parametros:     s: cadena a normalizar (se modifica in place).
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void arreglar_apyn_ALU(char *s)
{
    if (!s) return;
    char temp[MAX_STR] = {0};
    char* p_src = s;
    char* p_dst = temp;
    int coma_puesta = 0;
    int nueva_palabra = 1;
    while (*p_src != '\0' && (p_dst - temp) < MAX_STR - 2) {

        if (*p_src == ',') {
            if (!coma_puesta) {

                if (p_dst > temp && *(p_dst - 1) != ' ') {
                    *p_dst++ = ',';
                    *p_dst++ = ' ';
                    coma_puesta = 1;
                    nueva_palabra = 1;
                }
            }
        } else if (isspace(*p_src)) {
            if (!coma_puesta) {
                if (p_dst > temp && *(p_dst - 1) != ' ') {
                    *p_dst++ = ',';
                    *p_dst++ = ' ';
                    coma_puesta = 1;
                    nueva_palabra = 1;
                }
            } else {
                if (p_dst > temp && *(p_dst - 1) != ' ') {

                    *p_dst++ = ' ';
                    nueva_palabra = 1;
                }
            }
        } else {
            if (nueva_palabra) {
                *p_dst++ = toupper(*p_src);
                nueva_palabra = 0;
            } else *p_dst++ = tolower(*p_src);
        }
        p_src++;
    }
    if (p_dst > temp && *(p_dst - 1) == ' ') p_dst--;
    if (p_dst > temp && *(p_dst - 1) == ',') p_dst--;

    *p_dst = '\0';
    p_src = s;
    p_dst = temp;
    while (*p_dst != '\0') *p_src++ = *p_dst++;

    *p_src = '\0';
}

/******************************************************************************
Descripcion:    separa una linea CSV in place por SEPARADOR_CSV, reemplazando
                cada separador por '\0' y guardando en campos[] el puntero al
                inicio de cada campo.
Parametros:     linea: linea a separar (se modifica).
                campos: vector donde se guardan los punteros a cada campo.
                max: cantidad maxima de campos a separar.
Retorno:        Cantidad de campos encontrados (los vacios cuentan). Nunca mas
                de max.
Observaciones:  Si la linea tiene mas separadores que max, el ultimo campo
                conserva el resto de la linea.
******************************************************************************/
int separar_campos_ALU(char *linea, char *campos[], int max)
{
    if (linea == NULL || campos == NULL || max <= 0) {
        return 0;
    }

    int cant = 0;


    campos[cant] = linea;
    cant++;
    char *p = linea;

    while (*p != '\0') {

        if (*p == SEPARADOR_CSV) {


            if (cant < max) {
                *p = '\0';
                campos[cant] = p + 1;
                cant++;
            } else {

                break; // el ultimo queda con el resto de la linea.
            }
        }
        p++;
    }

    return cant;
}

/******************************************************************************
MIS FUNCIONES
******************************************************************************/
void cadena_a_mayusculas_ALU(char *cadena)
{
    if (cadena == NULL) {
        return;
    }

    while (*cadena != '\0') {
        *cadena = (char)AMAYUS(*cadena);
        cadena++;
    }
}

void reemplazar_caracter(char *cadena, char a , char b)
{
    for (int i = 0; cadena[i] != '\0'; i++) {
        if (cadena[i] == a) {
            cadena[i] = b;
        }
    }
}
