#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fecha.h"
#include "util.h"
#include "indice.h"      /* OK / ERROR */

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/



/******************************************************************************
Descripcion:    solicita la fecha de proceso hasta obtener una valida. Si el
                usuario solo presiona Enter toma la fecha actual del sistema.
Parametros:     fecha_proceso: fecha destino.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
int obtener_fecha_actual_ALU(t_fecha *fecha)
{
    time_t ahora = time(NULL);
    struct tm *fecha_local = localtime(&ahora);

    if (fecha_local == NULL) {
        return ERROR;
    }

    fecha->dia = fecha_local->tm_mday;
    fecha->mes = fecha_local->tm_mon + 1;
    fecha->anio = fecha_local->tm_year + 1900;
    return OK;
}

void pedir_fecha_proceso_ALU(t_fecha *fecha_proceso)
{
    char input[20];
    char caracter_extra;
    t_fecha fecha;

    if (fecha_proceso == NULL) {
        printf("Error: fecha de proceso invalida.\n");
        return;
    }

    fecha_proceso->dia = 0;
    fecha_proceso->mes = 0;
    fecha_proceso->anio = 0;

    while (1) {
        printf("Ingrese la fecha de proceso (DD/MM/AAAA) [Enter = fecha actual]: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nError: no se pudo leer la fecha de proceso.\n");
            return;
        }

        if (input[0] == '\n') {
            obtener_fecha_actual_ALU(fecha_proceso);
            return;
        }

        if (sscanf(input, "%d/%d/%d %c", &fecha.dia, &fecha.mes, &fecha.anio,&caracter_extra) != 3 || !fecha_correcta(&fecha) ) {
            printf("FECHA FORMALMENTE INCORRECTA.\n");
        }  else {
            *fecha_proceso = fecha;
            return;
        }
    }
}
