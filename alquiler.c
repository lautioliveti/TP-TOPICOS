#include <stdlib.h>
#include <string.h>
#include "alquiler.h"
#include "cadena.h"
#include "util.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    imprime por pantalla la matriz de consumo con totales por fila
                (plan) y por columna (genero).
Parametros:     m: matriz de consumo.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void tablero_mostrar_ALU(t_matriz_consumo m)
{
    int totales_genero[CANT_GENEROS] = {0};
    int total_general = 0;

    printf("%-15s", "PLAN");
    for (int j = 0; j < CANT_GENEROS; j++) {
        printf("%15s", GENEROS[j]);
    }
    printf("%10s\n", "TOTAL");

    for (int i = 0; i < CANT_PLANES; i++) {
        int total_plan = 0;

        printf("%-15s", PLANES[i]);
        for (int j = 0; j < CANT_GENEROS; j++) {
            printf("%15d", m[i][j]);
            total_plan += m[i][j];
            totales_genero[j] += m[i][j];
        }
        printf("%10d\n", total_plan);
    }

    printf("%-15s", "TOTAL");
    for (int j = 0; j < CANT_GENEROS; j++) {
        printf("%15d", totales_genero[j]);
        total_general += totales_genero[j];
    }
    printf("%10d\n", total_general);
}

/******************************************************************************
Descripcion:    ejecuta el flujo de alquiler de un titulo: 1) habilitacion del
                socio (debe estar en el indice), 2) inventario (el titulo debe
                estar en el indice y con stock > 0), 3) descuenta una unidad
                de stock en el archivo, 4) incrementa la celda [plan][genero]
                de la matriz, 5) emite el comprobante.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                path_socios: ruta de socios.dat.
                path_titulos: ruta de titulos.dat.
                matriz: matriz de consumo a alimentar.
                fecha_proceso: fecha de proceso para el comprobante.
Retorno:        OK si el alquiler se concreto, ERROR si alguna etapa fallo.
Observaciones:  Si una etapa falla se informa el motivo y no se ejecutan las
                siguientes. No se exige cuota al dia.
******************************************************************************/
int alquilar_ALU(t_indice *ind_socios, t_indice *ind_titulos, const char *path_socios,
             const char *path_titulos, t_matriz_consumo matriz_consumo, const t_fecha *fecha_proceso)
{
    FILE *arch_socios;
    FILE *arch_titulos;
    t_reg_indice clave_socio;
    t_reg_indice clave_titulo;
    t_socio socio;
    t_titulo titulo;
    long dni = 0;
    int id_titulo = 0;
    int plan;
    int genero;
    int posicion;

    printf("DNI del socio: ");
    if (scanf("%ld", &dni) != 1) {
        limpiar_entrada();
        printf("Error: DNI invalido.\n");
        return ERROR;
    }
    limpiar_entrada();

    clave_socio.id = dni;
    clave_socio.nro_reg = 0;
    posicion = indice_buscar_ALU(ind_socios, &clave_socio, sizeof(clave_socio),compara_reg_indice_ALU);

    if (posicion == NO_EXISTE) {
        printf("Error no existe un socio activo con DNI %ld.\n", dni);
        return ERROR;
    }

    arch_socios = fopen(path_socios, "rb");
    if (!arch_socios) {
        printf("Error no se pudo abrir %s.\n", path_socios);
        return ERROR;
    }

    if (socio_leer_ALU(arch_socios, clave_socio.nro_reg, &socio) != OK)
    {
        printf("Error no se pudo leer el socio de %s.\n", path_socios);
        fclose(arch_socios);
        return ERROR;
    }
    fclose(arch_socios);

    printf("ID del titulo a alquilar: ");
    if (scanf("%d", &id_titulo) != 1) {
        limpiar_entrada();
        printf("Error: ID de titulo invalido.\n");
        return ERROR;
    }
    limpiar_entrada();

    clave_titulo.id = id_titulo;
    clave_titulo.nro_reg = 0;

    posicion = indice_buscar_ALU(ind_titulos, &clave_titulo, sizeof(clave_titulo),
                                 compara_reg_indice_ALU);
    if (posicion == NO_EXISTE) {
        printf("Error no existe un titulo activo con ID %d.\n", id_titulo);
        return ERROR;
    }

    arch_titulos = fopen(path_titulos, "r+b");
    if (!arch_titulos ) {
        printf("Error no se pudo abrir %s.\n", path_titulos);
        return ERROR;
    }

    if (titulo_leer(arch_titulos, clave_titulo.nro_reg, &titulo) != OK) {
        printf("Error: no se pudo leer el titulo de %s.\n", path_titulos);
        fclose(arch_titulos);
        return ERROR;
    }

    if (titulo.stock <= 0) {
        printf("Error no hay stock disponible del titulo \"%s\".\n", titulo.titulo);
        fclose(arch_titulos);
        return ERROR;
    }

    plan = plan_indice(socio.plan);
    genero = genero_indice(titulo.genero);

    if (plan < 0 || plan >= CANT_PLANES || genero < 0 || genero >= CANT_GENEROS)
    {
        printf("Error plan del socio o genero del titulo no valido.\n");
        fclose(arch_titulos);
        return ERROR;
    }

    titulo.stock--;
    if (titulo_escribir(arch_titulos, clave_titulo.nro_reg, &titulo) != OK)
    {
        printf("Error no se pudo actualizar el stock del titulo en %s.\n", path_titulos);
        fclose(arch_titulos);
        return ERROR;
    }

    fclose(arch_titulos);


    matriz_consumo[plan][genero]++;

    printf("\n========== COMPROBANTE DE ALQUILER ==========\n");
    printf("Fecha: %02d/%02d/%04d\n", fecha_proceso->dia, fecha_proceso->mes, fecha_proceso->anio);
    printf("Socio: %ld - %s (plan %s)\n", socio.dni, socio.apellidos_nombres, socio.plan);
    printf("Titulo: %d - %s (%s)\n", titulo.id, titulo.titulo, titulo.genero);
    printf("Stock restante: %d\n", titulo.stock);
    printf("=============================================\n");
    return OK;
}
