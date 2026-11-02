#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "validaciones.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    calcula el CUIL con el algoritmo modulo 11 del Anexo A.1:
                prefijo 20 (hombre) o 27 (mujer), DNI a 8 digitos y digito
                verificador. Si el resto es 1 el prefijo pasa a 23 y Z es 9
                (hombre) o 4 (mujer).
Parametros:     dni: numero de documento.
                sexo: 'H' o 'M'.
                dest: buffer de al menos TAM_CUIL bytes donde se deja "XY-
                NNNNNNNN-Z".
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void calcular_cuil_ALU(long dni, char sexo, char *dest)
{
    int xy = 0;
    int z = 0;
    int resto, suma = 0;
    int multiplicadores[10] = {5, 4, 3, 2, 7, 6, 5, 4, 3, 2};


    int digitos[10];


    if (sexo == 'H' || sexo == 'h') {
        xy = 20;
    } else if (sexo == 'M' || sexo == 'm') {
        xy = 27;
    }


    digitos[0] = xy / 10;
    digitos[1] = xy % 10;

    char dni_str[9];
    sprintf(dni_str, "%08ld", dni);

    for (int i = 0; i < 8; i++) {
        digitos[i + 2] = dni_str[i] - '0';
    }

    for (int i = 0; i < 10; i++) {
        suma += digitos[i] * multiplicadores[i];
    }
    resto = suma % 11;


    if (resto == 0) {
        z = 0;
    } else if (resto == 1) {
        if (xy == 20) { // Hombre
            xy = 23;
            z = 9;
        } else if (xy == 27) { // Mujer
            xy = 23;
            z = 4;
        }
    } else {
        z = 11 - resto;
    }

    sprintf(dest, "%02d-%08ld-%d", xy, dni, z);
}

/******************************************************************************
Descripcion:    valida un email segun el Anexo A.3: hasta 30 caracteres sin
                espacios, una unica '@', parte local de letras/digitos/puntos
                que no empieza ni termina con punto, y dominio de
                letras/digitos/guiones/puntos con al menos un punto, que no
                empieza ni termina con punto o guion y cuyo ultimo segmento
                tiene al menos 2 letras.
Parametros:     email: cadena a validar.
Retorno:        1 si es valido, 0 en caso contrario.
Observaciones:  No verifica unicidad; eso lo hace socio_email_existe.
******************************************************************************/
int email_valido_ALU(const char *email)
{
    if (!email) return 0;

    const char *p = email;
    int total_len = 0;
    int local_len = 0;
    int puntos_dominio = 0;
    int letras_ultimo_segmento = 0;
    int es_letra, es_dig;

    // Regla 3
    if (*p == '.') return 0;


    while (*p && *p != '@') {
        total_len++;
        if (total_len > 30) return 0; // Regla 1
        if (*p == ' ') return 0;      // Regla 1

        es_letra = ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'));
        es_dig = (*p >= '0' && *p <= '9');

        // Regla 3
        if (!es_letra && !es_dig && *p != '.') return 0;

        local_len++;
        p++;
    }

    // Regla 2
    if (*p != '@') return 0;

    // Regla 3
    if (local_len == 0) return 0;
    if (*(p - 1) == '.') return 0;

    // Avanzamos para saltar la '@'
    total_len++;
    if (total_len > 30)
        return 0;
    p++;

    // Regla 4
    if (*p == '\0' || *p == '.' || *p == '-')
        return 0;

    // @ dsps
    while (*p) {
        total_len++;
        if (total_len > 30) return 0; // Regla 1
        if (*p == ' ') return 0;      // Regla 1

        es_letra = ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'));
        es_dig = (*p >= '0' && *p <= '9');

        // Regla 4
        if (!es_letra && !es_dig && *p != '.' && *p != '-') return 0;

        // Validaciones del segmento final
        if (*p == '.') {
            puntos_dominio++;
            letras_ultimo_segmento = 0; // Se resetea al encontrar un nuevo punto
        } else if (es_letra) {
            letras_ultimo_segmento++;
        }

        p++;
    }

    // Regla 4
    if (puntos_dominio == 0) return 0;


    if (*(p - 1) == '.' || *(p - 1) == '-') return 0;


    if (letras_ultimo_segmento < 2) return 0;

    return 1;
}
