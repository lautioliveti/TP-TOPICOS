#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "indice.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/
/*


*//// MI FUNCION DE BUSQUEDA BINARIA AUX
int binary_search_ALU(const void *vector, const void *elemento, size_t n, size_t tamanyo, int (*cmp)(const void *, const void *))
{
    int ini = 0;
    int fin = (int)n - 1;
    int medio, comp;
    const char *base = (const char *)vector;

    while (ini <= fin)
    {
        medio = ini + (fin - ini) / 2;
        comp = cmp(elemento, base + (size_t)medio * tamanyo);

        if (comp == 0)
            return medio;
        else if (comp < 0)
            fin = medio - 1;
        else
            ini = medio + 1;
    }

    return NO_EXISTE;
}



/******************************************************************************
Descripcion:    toma memoria para nmemb elementos e inicializa la estructura
                vacia.
Parametros:     indice: TDA indice.
                nmemb: cantidad inicial de elementos (CANTIDAD_ELEMENTOS).
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int indice_crear_ALU(t_indice *indice, size_t nmemb, size_t tamanyo)
{
    indice->cantidad_elementos_maxima = nmemb;
    indice->cantidad_elementos_actual = 0;
    indice->vindice = (void *)malloc(nmemb * tamanyo);
    if(indice->vindice == NULL)
    {
        indice->cantidad_elementos_maxima = 0;
        return ERROR;
    }
    return OK;
}

/******************************************************************************
Descripcion:    redimensiona el arreglo interno del indice con realloc. La
                nueva capacidad es la actual multiplicada por INCREMENTO (30%
                mas); la calcula la propia funcion.
Parametros:     indice: TDA indice.
                tamanyo: espacio en bytes ocupado por cada elemento.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Si el producto no aumenta la capacidad (arreglos muy chicos)
                se garantiza al menos un elemento mas.
******************************************************************************/
int indice_redimensionar_ALU(t_indice *indice, size_t tamanyo)
{
    size_t nueva_capacidad = indice->cantidad_elementos_maxima * INCREMENTO;


    if (nueva_capacidad <= indice->cantidad_elementos_maxima)
        nueva_capacidad = indice->cantidad_elementos_maxima + 1;

    void* nuevoIndice = (void*)realloc(indice->vindice, nueva_capacidad * tamanyo);

    if(!nuevoIndice)
        return ERROR;

    indice->vindice = nuevoIndice;
    indice->cantidad_elementos_maxima = nueva_capacidad;
    return OK;
}

/******************************************************************************
Descripcion:    inserta un elemento manteniendo el orden segun la clave. Si el
                arreglo esta lleno invoca indice_redimensionar.
Parametros:     indice: TDA indice.
                registro: elemento a insertar.
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario
                (clave duplicada o falta de memoria).
Observaciones:  -
******************************************************************************/
int indice_insertar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                    int (*cmp)(const void *, const void *))
{
    int limiteIzquierdo = 0;
    int limiteDerecho = (int)indice->cantidad_elementos_actual - 1;
    int comp, medio;
    int duplicado = 0;
    int res = OK;
    char *vector;

    if (indice->cantidad_elementos_actual == indice->cantidad_elementos_maxima)
    {
        res = indice_redimensionar_ALU(indice, tamanyo);
    }

    if (res == OK)
    {
        vector = (char *)indice->vindice;

        while (limiteIzquierdo <= limiteDerecho && !duplicado)
        {
            medio = limiteIzquierdo + (limiteDerecho - limiteIzquierdo) / 2;
            comp = cmp(registro, vector + (size_t)medio * tamanyo);

            if (comp == 0)
                duplicado = 1;
            else if (comp > 0)
                limiteIzquierdo = medio + 1;
            else
                limiteDerecho = medio - 1;
        }

        if (duplicado)
        {
            res = ERROR;
        }
        else
        {
            memmove(vector + (size_t)(limiteIzquierdo + 1) * tamanyo,
                    vector + (size_t)limiteIzquierdo * tamanyo,
                    (indice->cantidad_elementos_actual - (size_t)limiteIzquierdo) * tamanyo);
            memcpy(vector + (size_t)limiteIzquierdo * tamanyo, registro, tamanyo);
            indice->cantidad_elementos_actual++;
        }
    }

    return res;
}
/******************************************************************************
Descripcion:    elimina el elemento cuya clave coincide con la de registro,
                desplazando los siguientes.
Parametros:     indice: TDA indice.
                registro: elemento a eliminar (solo importa su clave).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR si la clave no existe.
Observaciones:  -
******************************************************************************/
int indice_eliminar_ALU(t_indice *indice, const void *registro, size_t tamanyo,
                    int (*cmp)(const void *, const void *))
{
    int posicion;
    char *vector;
    int res = ERROR;

    if (indice->cantidad_elementos_actual > 0)
    {
        posicion = binary_search_ALU(indice->vindice, registro,
                                     indice->cantidad_elementos_actual, tamanyo, cmp);
        if (posicion != NO_EXISTE)
        {
            vector = (char *)indice->vindice;
            memmove(vector + (size_t)posicion * tamanyo,
                    vector + (size_t)(posicion + 1) * tamanyo,
                    (indice->cantidad_elementos_actual - (size_t)posicion - 1) * tamanyo);
            indice->cantidad_elementos_actual--;
            res = OK;
        }
    }
    return res;
}
/******************************************************************************
Descripcion:    busca la clave de registro en el indice. Si existe copia el
                elemento completo (nro_reg incluido) en registro.
Parametros:     indice: TDA indice.
                registro: elemento a buscar (entrada/salida).
                tamanyo: espacio en bytes ocupado por el elemento.
                cmp: funcion de comparacion provista.
Retorno:        NO_EXISTE si la clave no existe; en caso contrario la posicion
                que ocupa dentro del arreglo.
Observaciones:  La comparacion entre claves se realiza exclusivamente mediante
                cmp.
******************************************************************************/
int indice_buscar_ALU(const t_indice *indice, void *registro, size_t tamanyo,
                  int (*cmp)(const void *, const void *))
{
    int pos = NO_EXISTE;

    if (indice->cantidad_elementos_actual > 0)
    {
        pos = binary_search_ALU(indice->vindice, registro,
                                indice->cantidad_elementos_actual, tamanyo, cmp);
        if (pos != NO_EXISTE)
            memcpy(registro, (const char *)indice->vindice + (size_t)pos * tamanyo, tamanyo);
    }
    return pos;
}

/******************************************************************************
Descripcion:    determina si el indice contiene 0 (cero) elementos.
Parametros:     indice: TDA indice.
Retorno:        OK si esta vacio, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_vacio_ALU(const t_indice *indice)
{
    if (indice->cantidad_elementos_actual == 0)
        return OK;
    return ERROR;
}

/******************************************************************************
Descripcion:    determina si el indice alcanzo su capacidad maxima actual.
Parametros:     indice: TDA indice.
Retorno:        OK si esta lleno, ERROR si no lo esta.
Observaciones:  -
******************************************************************************/
int indice_lleno_ALU(const t_indice *indice)
{
    if (indice->cantidad_elementos_actual == indice->cantidad_elementos_maxima)
        return OK;
    return ERROR;
}

/******************************************************************************
Descripcion:    deja el indice vacio sin liberar la memoria del arreglo.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void indice_vaciar_ALU(t_indice *indice)
{
   indice->cantidad_elementos_actual = 0;
}

/******************************************************************************
Descripcion:    libera toda la memoria dinamica del indice.
Parametros:     indice: TDA indice.
Retorno:        No posee.
Observaciones:  Luego de invocarla el indice no puede usarse sin volver a
                llamar a indice_crear.
******************************************************************************/
void indice_destruir_ALU(t_indice *indice)
{
    if(indice != NULL)
    {
        free(indice->vindice);
        indice->vindice = NULL;
        indice->cantidad_elementos_actual = 0;
        indice->cantidad_elementos_maxima = 0;
    }
}

/******************************************************************************
Descripcion:    construye el indice a partir de un archivo binario de
                registros. Recorre el archivo secuencialmente y por cada
                registro activo inserta un t_reg_indice (nro_reg = posicion en
                el archivo, id = clave obtenida con obtener_clave).
Parametros:     path: ruta al archivo binario.
                indice: TDA indice (ya creado; se vacia antes de cargar).
                tamanyo_reg: tamanyo en bytes de cada registro del archivo.
                obtener_clave: funcion que devuelve la clave (long) de un
                registro.
                es_activo: funcion que devuelve distinto de 0 si el registro
                tiene estado 'A'.
                cmp: funcion de comparacion provista.
Retorno:        OK si la operacion fue exitosa y ERROR en caso contrario.
Observaciones:  Los registros con estado 'B' se omiten. El archivo no necesita
                estar ordenado: el orden lo garantiza indice_insertar.
******************************************************************************/
int indice_cargar_ALU(const char *path, t_indice *indice, size_t tamanyo_reg,
                  long (*obtener_clave)(const void *),
                  int (*es_activo)(const void *),
                  int (*cmp)(const void *, const void *))
{

    FILE *pfArchivo = fopen(path, "rb");
    if (!pfArchivo)
        return ERROR;


    indice_vaciar_ALU(indice);


    void *buffer_reg = malloc(tamanyo_reg);
    if (!buffer_reg)
    {
        fclose(pfArchivo);
        return ERROR;
    }

    t_reg_indice reg_indice;
    unsigned nro_reg_archivo = 0;
    int res= OK;
    while ( res == OK && fread(buffer_reg, tamanyo_reg, 1, pfArchivo) == 1)
    {

        if (es_activo(buffer_reg) != 0)
        {

            reg_indice.id = obtener_clave(buffer_reg);
            reg_indice.nro_reg = nro_reg_archivo;


            if(indice_insertar_ALU(indice, &reg_indice, sizeof(t_reg_indice), cmp) != OK)
            {
                res = ERROR;
            }
        }


        nro_reg_archivo++;
    }

    free(buffer_reg);
    fclose(pfArchivo);

    return res;
}
