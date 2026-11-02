#include <string.h>
#include "util.h"
#include "cadena.h"
#include "config.h"

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    opcion Salir: copia socios.dat y titulos.dat con la fecha de
                proceso en el nombre, vuelca la matriz de consumo a disco y
                libera la memoria de ambos indices.
Parametros:     ind_socios: indice de socios.
                ind_titulos: indice de titulos.
                matriz: matriz de consumo.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void salir_y_persistir_ALU(t_indice *ind_socios, t_indice *ind_titulos,
                              t_matriz_consumo matriz_consumo, const t_fecha *fecha_proceso)
{
    char fecha_compacta[9];
    char path_socios[TAM_LINEA];
    char path_titulos[TAM_LINEA];
    char path_matriz[TAM_LINEA];


    fecha_a_texto_compacto(fecha_proceso, fecha_compacta);

    sprintf(path_socios, "%s%s.dat",PREFIJO_SOCIOS_BKP, fecha_compacta);
    if(archivo_copiar(ARCH_SOCIOS_BIN, path_socios) != OK) {
        printf("Error no se pudo guardar el respaldo de socios en %s.\n",path_socios);
    } else {
        printf("%s -> %s\n", ARCH_SOCIOS_BIN, path_socios);
    }

    sprintf(path_titulos, "%s%s.dat", PREFIJO_TITULOS_BKP, fecha_compacta);
    if (archivo_copiar(ARCH_TITULOS_BIN, path_titulos) != OK) {
        printf("Error no se pudo guardar el respaldo de titulos en %s.\n",path_titulos);
    } else {
        printf("%s -> %s\n", ARCH_TITULOS_BIN, path_titulos);
    }

    sprintf(path_matriz,"%s%s.dat",PREFIJO_MATRIZ, fecha_compacta);
    if (matriz_consumo_persistir(matriz_consumo, path_matriz) != OK)
    {
        printf("Error no se pudo persistir la matriz de consumo en %s.\n",path_matriz);
    } else {
        printf("matriz de consumo -> %s\n", path_matriz);
    }

    indice_destruir_ALU(ind_socios);
    indice_destruir_ALU(ind_titulos);
    printf("Memoria liberada. Hasta luego.\n");

}
