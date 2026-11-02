#include <stdlib.h>
#include <string.h>
#include "titulo.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 4 campos en t_titulo. Los campos
                numericos invalidos quedan en valores que fallan la validacion
                (id = 0, stock = -1). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_titulo destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_titulo_ALU(const char *linea, void *reg)
{
    t_titulo *titulo;
    char buffer[TAM_LINEA];
    char *campos[CAMPOS_TITULO + 1];
    int cant_campos;

    titulo = (t_titulo *)reg;
    titulo->id = 0;
    titulo->titulo[0] = '\0';
    titulo->genero[0] = '\0';
    titulo->stock = -1;
    titulo->estado = 'A';

    strcpy(buffer, linea);

    char *salto_linea = buffer;
    while (*salto_linea != '\0' && *salto_linea != '\n')
    {
        salto_linea++;
    }
    if(*salto_linea == '\n')
        *salto_linea = '\0';

    cant_campos = separar_campos_ALU(buffer, campos, CAMPOS_TITULO + 1);
    if (cant_campos != CAMPOS_TITULO) {
        return ERROR;
    }

    if (sscanf(campos[0], "%d", &titulo->id) != 1) {
        titulo->id = 0;
    }

    strcpy(titulo->titulo, campos[1]);
    strcpy(titulo->genero, campos[2]);

    if (sscanf(campos[3], "%d", &titulo->stock) != 1) {
        titulo->stock = -1;
    }

    return OK;
}

/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.2 en orden: ID > 0 y no
                existente en el indice, titulo normalizado no vacio, genero
                dentro de GENEROS[] y stock >= 0. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_titulo (se modifica al normalizar el titulo).
                ctx: contexto con indice e id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el titulo es valido, ERROR en caso contrario.
Observaciones:  Se pasa por puntero como validar a leer_archivo_texto. Se reutiliza en las altas del
                lote de novedades.
******************************************************************************/
int valida_titulo_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_titulo *titulo = (t_titulo *)reg;
    t_reg_indice clave;
    int i;
    int genero_valido = 0;

    if (titulo->id <= 0) {
        strcpy(tipo_error, "ID");
        return ERROR;
    }

    if (titulo->id != ctx->id_modificando) {
        clave.id = titulo->id;
        clave.nro_reg = 0;
        if (indice_buscar_ALU(ctx->indice, &clave, sizeof(clave),compara_reg_indice_ALU) != NO_EXISTE) {
            strcpy(tipo_error, "ID");
            return ERROR;
        }
    }

    arreglar_titulo(titulo->titulo);
    if (cadena_vacia(titulo->titulo)) {
        strcpy(tipo_error, "Titulo");
        return ERROR;
    }

    for (i = 0; i < CANT_GENEROS; i++) {
        if (strcmp(titulo->genero, GENEROS[i]) == 0) {
            genero_valido = 1;
            break;
        }
    }
    if (!genero_valido) {
        strcpy(tipo_error, "Genero");
        return ERROR;
    }

    if (titulo->stock < 0) {
        strcpy(tipo_error, "Stock");
        return ERROR;
    }

    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por ID y solo con activos) y lista
                los titulos leyendo cada registro del archivo.
Parametros:     indice: indice de titulos.
                path: ruta de titulos.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int titulos_listar_ALU(const t_indice *indice, const char *path)
{
    t_reg_indice *registros_indice;
    t_titulo titulo;

    FILE *arch = fopen(path, "rb");

    if (!arch) {
        printf("Error no se pudo abrir %s.\n", path);
        return ERROR;
    }

    registros_indice = (t_reg_indice *)indice->vindice;
    printf("%-6s %-40s %-15s %7s %s\n","ID", "TITULO", "GENERO", "STOCK", "E");

    unsigned i = 0;
    while( i < indice->cantidad_elementos_actual)
    {
        if (titulo_leer(arch, registros_indice[i].nro_reg, &titulo) != OK) {
            printf("Error no se pudo leer el titulo %ld de %s.\n",registros_indice[i].id, path);
            fclose(arch);
            return ERROR;
        }
        titulo_mostrar(&titulo);
        i++;
    }

    printf("\nTotal de titulos activos: %u\n",indice->cantidad_elementos_actual);

    fclose(arch);
    return OK;
}
