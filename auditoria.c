#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auditoria.h"
#include "lectura.h"
#include "cadena.h"
#include "util.h"
#include "indice.h"



#define CANT_INCIDENCIAS_INICIAL 8

typedef struct
{
    char dni[TAM_LINEA];
    char tipo_error[TAM_LINEA];
    unsigned long cantidad;
} t_incidencia;

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

int buscar_incidencia(t_incidencia *incidencias, size_t cantidad, const char *dni, const char *tipo_error)
{

    for (int i = 0; i <(int)cantidad; i++) {
        if (strcmp(incidencias[i].dni, dni) == 0 && strcmp(incidencias[i].tipo_error, tipo_error) == 0) {
            return i;
        }
    }
    return -1;
}


/******************************************************************************
Descripcion:    agrega al final del archivo de auditoria una linea con el
                formato DNI;TIPO_VALIDACION;FECHA_PROCESO.
Parametros:     path: ruta del archivo de texto de errores.
                dni_texto: DNI tal como vino en la entrada.
                tipo_error: nombre de la validacion que fallo.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  El archivo se abre en modo append; si no existe se crea.
******************************************************************************/
int auditoria_registrar_ALU(const char *path, const char *dni_texto, const char *tipo_error, const t_fecha *fecha_proceso)
{
    FILE* arch = fopen(path,"a");
    if(!arch)
    {
        printf("Error al abrir archivo %s", path);
        return ERROR;
    }

    fprintf(arch,"%s;%s;%d/%d/%d\n", dni_texto,tipo_error,fecha_proceso->dia,fecha_proceso->mes,fecha_proceso->anio);

    fclose(arch);
    return OK;
}

/******************************************************************************
Descripcion:    lee el archivo de auditoria y muestra, por cada par (DNI, tipo
                de validacion), la cantidad de incidencias registradas y el
                total general.
Parametros:     path: ruta del archivo de texto de errores.
Retorno:        OK si se pudo procesar (aunque el archivo no exista), ERROR si
                fallo la memoria.
Observaciones:  Acumula las incidencias en un vector dinamico reservado con
                malloc.
******************************************************************************/
int auditoria_mostrar_ALU(const char *path)
{
    FILE *arch;
    char linea[TAM_LINEA];
    int cantidad_incidencias = 0;
    size_t capacidad = CANT_INCIDENCIAS_INICIAL;
    unsigned long total = 0;

    arch = fopen(path, "rt");
    if (!arch) {
        printf("Error: no se pudo abrir %s.\n", path);
        return ERROR;
    }

    t_incidencia *incidencias = malloc(capacidad * sizeof(t_incidencia));
    if (!incidencias) {
        printf("Error: no hay memoria para procesar la auditoria.\n");
        fclose(arch);
        return ERROR;
    }

    char dni[TAM_LINEA] = "";
    char tipo_error[TAM_LINEA] = "";
    char fecha[TAM_LINEA] = "";
    int indice_encontrado;

    while (fgets(linea, TAM_LINEA, arch))
    {

        sscanf(linea, "%[^;];%[^;];%[^\n]", dni, tipo_error, fecha);

        cadena_a_mayusculas_ALU(tipo_error);
        reemplazar_caracter(tipo_error,' ','_');

        indice_encontrado = buscar_incidencia(incidencias, cantidad_incidencias, dni, tipo_error);

        if (indice_encontrado != -1)
        {

            incidencias[indice_encontrado].cantidad++;
            total++;
        }else
        {
            //chequeamos si se alcanzo al maximo de incidencias
            if ((size_t)cantidad_incidencias == capacidad)
            {

                size_t nueva_capacidad = capacidad * INCREMENTO;
                t_incidencia *nuevas_incidencias = realloc(incidencias, nueva_capacidad * sizeof(t_incidencia));

                if (!nuevas_incidencias)
                {
                    printf("Error: no hay memoria para procesar la auditoria.\n");
                    free(incidencias);
                    fclose(arch);
                    return ERROR;
                }

                incidencias = nuevas_incidencias;
                capacidad = nueva_capacidad;
            }

            // nuevo reg agregando
            strcpy(incidencias[cantidad_incidencias].dni, dni);
            strcpy(incidencias[cantidad_incidencias].tipo_error, tipo_error);
            incidencias[cantidad_incidencias].cantidad = 1;

            cantidad_incidencias++;
            total++;
        }

    }

    fclose(arch);

    printf("%-15s %-23s %8s\n", "DNI", "TIPO DE VALIDACION", "CANTIDAD");
    printf("--------------- ----------------------- --------\n");

    for (int i = 0; i < (int)cantidad_incidencias; i++) {
        printf("%-15s %-23s %8lu\n",incidencias[i].dni,incidencias[i].tipo_error,incidencias[i].cantidad);
    }

    printf("\nTotal de incidencias: %lu\n", total);
    free(incidencias);
    return OK;
}
