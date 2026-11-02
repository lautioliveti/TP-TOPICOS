#include <string.h>
#include <limits.h>
#include "novedad.h"
#include "cadena.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    recorre secuencialmente novedades_catalogo.dat y aplica cada
                operacion sobre el indice y titulos.dat: 'A' agrega si el ID
                no esta en el indice (validando como en la carga), 'B' pone
                estado 'B' en el archivo y elimina del indice, 'M' reemplaza
                titulo y genero y suma el stock (rechaza si quedaria
                negativo). Al final muestra el reporte de procesadas y
                rechazadas.
Parametros:     indice: indice de titulos.
                path_novedades: ruta de novedades_catalogo.dat.
                path_titulos: ruta de titulos.dat.
                fecha_proceso: fecha de proceso (para el contexto de
                validacion).
Retorno:        OK si se pudieron abrir los archivos, ERROR en caso contrario.
Observaciones:  Como el indice solo tiene activos, un 'A' sobre un ID dado de
                baja se agrega como registro nuevo al final del archivo.
******************************************************************************/
int novedades_procesar_ALU(t_indice *indice, const char *path_novedades,
                       const char *path_titulos, const t_fecha *fecha_proceso)
{
    FILE *arch_novedades;
    FILE *arch_titulos;
    t_novedad novedad;
    t_titulo titulo;
    t_titulo titulo_actual;
    t_reg_indice clave;
    t_reg_indice registro_indice;
    t_contexto ctx;
    char tipo_error[TAM_TIPO_ERROR];
    unsigned procesadas = 0;
    unsigned rechazadas = 0;
    unsigned nro_reg;
    long posicion_archivo;
    int posicion;
    int resultado = OK;


    arch_novedades = fopen(path_novedades, "rb");

    if (!arch_novedades)
    {
        printf("Error no se pudo abrir %s.\n", path_novedades);
        return ERROR;
    }

    arch_titulos = fopen(path_titulos, "r+b");

    if (!arch_titulos)
    {
        printf("Error no se pudo abrir %s.\n", path_titulos);
        fclose(arch_novedades);
        return ERROR;
    }

    ctx.indice = indice;
    ctx.arch_bin = arch_titulos;
    ctx.fecha_proceso = fecha_proceso;

    printf("Procesando %s...\n\n", path_novedades);
    fread(&novedad, sizeof(t_novedad), 1, arch_novedades);

    while (!feof(arch_novedades))
    {
        int novedad_valida = 1;
        const char *motivo_rechazo = NULL;

        switch (novedad.codigo_operacion) {
            case OP_ALTA:
                titulo = novedad.titulo;
                titulo.estado = 'A';

                ctx.id_modificando = 0;
                if (valida_titulo_ALU(&titulo, &ctx, tipo_error) != OK) {
                    if (strcmp(tipo_error, "ID") == 0) {
                        clave.id = titulo.id;
                        clave.nro_reg = 0;
                        motivo_rechazo =
                            indice_buscar_ALU(indice, &clave, sizeof(clave),compara_reg_indice_ALU) != NO_EXISTE? "el ID ya existe" : "ID";
                    } else {
                        cadena_a_mayusculas_ALU(tipo_error);
                        motivo_rechazo = tipo_error;
                    }
                    novedad_valida = 0;
                    break;
                }

                fseek(arch_titulos, 0, SEEK_END);
                posicion_archivo = ftell(arch_titulos);
                nro_reg = (unsigned)(posicion_archivo / (long)sizeof(t_titulo));
                fwrite(&titulo, sizeof(t_titulo), 1, arch_titulos);

                registro_indice.id = titulo.id;
                registro_indice.nro_reg = nro_reg;
                if (indice_insertar_ALU(indice, &registro_indice,sizeof(registro_indice),compara_reg_indice_ALU) != OK) {
                    titulo.estado = 'B';
                    if (titulo_escribir(arch_titulos, nro_reg, &titulo) != OK)
                    {
                        printf("Error no se pudo deshacer el alta en %s.\n",
                               path_titulos);
                        resultado = ERROR;
                    }
                    motivo_rechazo = "no se pudo actualizar el indice";
                    novedad_valida = 0;
                } else {
                    printf("  ALTA       id=%-4d   %s\n", titulo.id, titulo.titulo);
                }
                break;

            case OP_BAJA:
                clave.id = novedad.titulo.id;
                clave.nro_reg = 0;
                posicion = indice_buscar_ALU(indice, &clave, sizeof(clave),compara_reg_indice_ALU);
                if (posicion == NO_EXISTE) {
                    motivo_rechazo = "el ID no existe o ya esta dado de baja";
                    novedad_valida = 0;
                    break;
                }

                if (titulo_leer(arch_titulos, clave.nro_reg, &titulo_actual) != OK) {
                    printf("Error no se pudo leer el titulo de %s.\n", path_titulos);
                    motivo_rechazo = "error al leer el titulo";
                    resultado = ERROR;
                    novedad_valida = 0;
                    break;
                }

                titulo_actual.titulo[TAM_TITULO - 1] = '\0';
                titulo_actual.estado = 'B';
                if (titulo_escribir(arch_titulos, clave.nro_reg,  &titulo_actual) != OK) {
                    printf("Error no se pudo dar de baja el titulo en %s.\n", path_titulos);
                    motivo_rechazo = "error al dar de baja el titulo";
                    resultado = ERROR;
                    novedad_valida = 0;
                    break;
                }

                if (indice_eliminar_ALU(indice, &clave, sizeof(clave),compara_reg_indice_ALU) != OK) {
                    titulo_actual.estado = 'A';
                    if (titulo_escribir(arch_titulos, clave.nro_reg,&titulo_actual) != OK) {
                        printf("Error no se pudo restaurar el titulo en %s.\n",path_titulos);
                        resultado = ERROR;
                    }
                    motivo_rechazo = "no se pudo actualizar el indice";
                    novedad_valida = 0;
                } else {
                    printf("  BAJA       id=%-4d   %s\n",titulo_actual.id, titulo_actual.titulo);
                }
                break;

            case OP_MODIFICACION:
                clave.id = novedad.titulo.id;
                clave.nro_reg = 0;
                posicion = indice_buscar_ALU(indice, &clave, sizeof(clave),compara_reg_indice_ALU);
                if (posicion == NO_EXISTE) {
                    motivo_rechazo ="el ID no existe o ya esta dado de baja";
                    novedad_valida = 0;
                    break;
                }

                if (titulo_leer(arch_titulos, clave.nro_reg, &titulo_actual) != OK) {
                    motivo_rechazo = "error al leer el titulo";
                    resultado = ERROR;
                    novedad_valida = 0;
                    break;
                }

                strcpy(titulo_actual.titulo, novedad.titulo.titulo);
                strcpy(titulo_actual.genero, novedad.titulo.genero);

                if (titulo_actual.stock < 0) {
                    motivo_rechazo = "STOCK actual invalido";
                    novedad_valida = 0;
                    break;
                }
                if (0 > titulo_actual.stock + novedad.titulo.stock) {
                    motivo_rechazo = "STOCK (quedaria negativo)";
                    novedad_valida = 0;
                    break;
                }

                titulo_actual.stock += novedad.titulo.stock;
                ctx.id_modificando = titulo_actual.id;

                if (valida_titulo_ALU(&titulo_actual, &ctx, tipo_error) != OK)
                {
                    cadena_a_mayusculas_ALU(tipo_error);
                    motivo_rechazo = tipo_error;
                    novedad_valida = 0;
                    break;
                }

                if (titulo_escribir(arch_titulos, clave.nro_reg, &titulo_actual) != OK) {
                    motivo_rechazo = "error al modificar el titulo";
                    resultado = ERROR;
                    novedad_valida = 0;
                } else {
                    printf("  MODIFICADA id=%-4d   %s (stock %d)\n",  titulo_actual.id, titulo_actual.titulo, titulo_actual.stock);
                }
                break;

            default:
                motivo_rechazo = "codigo de operacion invalido";
                novedad_valida = 0;
                break;
        }

        if (novedad_valida) {
            procesadas++;
        } else
        {
            rechazadas++;
            printf("  RECHAZADA op=%c id=%-4d   motivo: %s\n", novedad.codigo_operacion, novedad.titulo.id,motivo_rechazo != NULL ? motivo_rechazo : "error desconocido");
        }
        // ERROR GRAVE = NO PODER ESCRIBIR, LEER UN TITULO SALE DEL WHILE
        if (resultado != OK) {
            break;
        }

        fread(&novedad, sizeof(t_novedad), 1, arch_novedades);
    }

    printf("\nNovedades procesadas: %u   Rechazadas: %u\n",procesadas, rechazadas);
    fclose(arch_novedades);
    fclose(arch_titulos);

    return resultado;
}
