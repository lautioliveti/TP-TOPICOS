#include <string.h>
#include "lectura.h"
#include "util.h"
#include "cadena.h"
#include "socio.h"
#include "titulo.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    funcion generica de lectura de archivos de texto. Por cada
                linea no vacia: parsear -> validar -> escribir al final del
                archivo binario e insertar en el indice. Ante el primer error
                (de formato o de validacion) se rechaza la linea invocando
                rechazar y se continua con la siguiente.
Parametros:     path_txt: ruta del archivo de texto (CSV).
                arch_bin: archivo binario destino abierto en "w+b" o "r+b".
                indice: indice de la entidad (se usa para validar duplicados y
                se va completando).
                reg: buffer de un registro de la entidad.
                tamanyo: tamanyo en bytes del registro.
                fecha_proceso: fecha de proceso.
                parsear: convierte una linea en registro.
                validar: valida el registro y deja el tipo de error.
                rechazar: accion ante un rechazo (auditar, ignorar, etc.).
                obtener_clave: devuelve la clave del registro para el indice.
                aceptados, rechazados: salida; contadores de registros.
Retorno:        OK si se pudo abrir el archivo de texto, ERROR en caso
                contrario.
Observaciones:  Se reutiliza para Socios.csv y Titulos.csv; lo especifico de
                cada entidad lo resuelven los punteros a funcion.
******************************************************************************/
int leer_archivo_texto_ALU(const char *path_txt, FILE *arch_bin, t_indice *indice,
                       void *reg, size_t tamanyo, const t_fecha *fecha_proceso,
                       int (*parsear)(const char *, void *),
                       int (*validar)(void *, const t_contexto *, char *),
                       void (*rechazar)(const void *, const char *, const char *, const t_fecha *),
                       long (*obtener_clave)(const void *),
                       unsigned *aceptados, unsigned *rechazados)
{
    char linea[TAM_LINEA];
    char tipo_error[TAM_TIPO_ERROR];
    t_contexto ctx;
    t_reg_indice reg_ind;
    long pos_final;
    FILE *arch_txt;
    int resultado = OK;

    *aceptados = 0;
    *rechazados = 0;

    arch_txt = fopen(path_txt, "rt");
    if (!arch_txt) {
        printf("Error al abrir archivo de texto %s.\n",path_txt);
        return ERROR;
    }

    ctx.indice = indice;
    ctx.arch_bin = arch_bin;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    while (fgets(linea, sizeof(linea), arch_txt))
    {
        quitar_salto(linea);
        if (cadena_vacia(linea)) {
            continue;
        }

        tipo_error[0] = '\0';
        if (parsear(linea, reg) != OK)
        {
            strcpy(tipo_error, "FORMATO");
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
            continue;
        }

        if (validar(reg, &ctx, tipo_error) != OK)
        {
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
            continue;
        }

        fseek(arch_bin, 0, SEEK_END);
        pos_final = ftell(arch_bin);

        reg_ind.nro_reg = (unsigned)(pos_final / (long)tamanyo);
        reg_ind.id = obtener_clave(reg);

        if (indice_insertar_ALU(indice, &reg_ind, sizeof(t_reg_indice), compara_reg_indice_ALU) != OK)
        {
            strcpy(tipo_error, "ERROR_INDICE");
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
            continue;
        }

        if (fwrite(reg, tamanyo, 1, arch_bin) != 1) {
            if (indice_eliminar_ALU(indice, &reg_ind, sizeof(t_reg_indice), compara_reg_indice_ALU) != OK) {
                printf("Error grave fallo la escritura y no se pudo quitar el registro del indice; pueden quedar inconsistentes.\n");
            }

            strcpy(tipo_error, "ERROR_ARCHIVO");
            rechazar(reg, linea, tipo_error, fecha_proceso);
            (*rechazados)++;
            resultado = ERROR;
            break;
        }

        fflush(arch_bin);

        (*aceptados)++;
    }

    fclose(arch_txt);

    return resultado;
}

/******************************************************************************
Descripcion:    compara dos t_reg_indice por su clave id.
Parametros:     a, b: punteros a t_reg_indice.
Retorno:        Negativo si a < b, 0 si son iguales, positivo si a > b.
Observaciones:  Se usa como cmp en todas las operaciones del TDA indice.
******************************************************************************/
int compara_reg_indice_ALU(const void *a, const void *b)
{
    const t_reg_indice *registro_a = (const t_reg_indice *)a;
    const t_reg_indice *registro_b = (const t_reg_indice *)b;

    if (registro_a->id < registro_b->id) {
        return -1;
    }
    if (registro_a->id > registro_b->id) {
        return 1;
    }
    return 0;
}

/******************************************************************************
Descripcion:    construye los indices de socios y de titulos a partir de los
                archivos binarios existentes (solo registros activos).
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
Retorno:        1 si ambos binarios existen y se cargaron, 0 en caso
                contrario.
Observaciones:  -
******************************************************************************/
int cargar_indices_ALU(t_indice *ind_socios, t_indice *ind_titulos)
{

    if (indice_cargar_ALU(ARCH_SOCIOS_BIN, ind_socios, sizeof(t_socio), clave_socio, activo_socio,compara_reg_indice_ALU) != OK ||
        indice_cargar_ALU(ARCH_TITULOS_BIN, ind_titulos, sizeof(t_titulo),clave_titulo, activo_titulo,compara_reg_indice_ALU) != OK)
    {
        indice_vaciar_ALU(ind_socios);
        indice_vaciar_ALU(ind_titulos);
        return 0;
    }

    return 1;
}

/******************************************************************************
Descripcion:    opcion 1 del menu: lee Socios.csv y Titulos.csv con la funcion
                generica de lectura, genera socios.dat y titulos.dat
                (sobrescribiendolos) y reconstruye los indices desde los
                binarios generados.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                fecha_proceso: fecha de proceso.
Retorno:        1 si los indices quedaron cargados, 0 en caso contrario.
Observaciones:  Los rechazos de socios se auditan en errores_socios.txt; los
                de titulos solo se cuentan.
******************************************************************************/
int generar_binarios_ALU(t_indice *ind_socios, t_indice *ind_titulos, const t_fecha *fecha_proceso)
{
    FILE *arch_socios;
    FILE *arch_titulos;

    t_socio socio;
    t_titulo titulo;

    unsigned socios_aceptados = 0;
    unsigned socios_rechazados = 0;
    unsigned titulos_aceptados = 0;
    unsigned titulos_rechazados = 0;

    int resultado_socios;
    int resultado_titulos;
    int resultado = OK;

    if (ind_socios == NULL || ind_titulos == NULL || fecha_proceso == NULL) {

        return ERROR;
    }

    arch_socios = fopen(ARCH_SOCIOS_BIN, "w+b");
    if (!arch_socios ) {

        return ERROR;
    }

    arch_titulos = fopen(ARCH_TITULOS_BIN, "w+b");
    if (!arch_titulos ) {

        fclose(arch_socios);
        return ERROR;
    }

    indice_vaciar_ALU(ind_socios);
    indice_vaciar_ALU(ind_titulos);

    resultado_socios = leer_archivo_texto_ALU(ARCH_SOCIOS_CSV, arch_socios, ind_socios, &socio,sizeof(t_socio), fecha_proceso, texto_a_socio_ALU, valida_socio_ALU,
        rechaza_socio, clave_socio, &socios_aceptados, &socios_rechazados);


    resultado_titulos = leer_archivo_texto_ALU(ARCH_TITULOS_CSV, arch_titulos, ind_titulos, &titulo,sizeof(t_titulo), fecha_proceso, texto_a_titulo_ALU,valida_titulo_ALU,
                                               rechaza_titulo, clave_titulo, &titulos_aceptados,&titulos_rechazados);

    fclose(arch_socios);
    fclose(arch_titulos);

    printf("Socios: %u aceptados, %u rechazados. (ver archivos/errores_socios.txt)\n",socios_aceptados, socios_rechazados);
    printf("Titulos: %u aceptados, %u rechazados.\n",titulos_aceptados, titulos_rechazados);

    if (resultado_socios != OK || resultado_titulos != OK)
    {
        resultado = ERROR;
    }

    return resultado;
}
