#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "socio.h"
#include "cadena.h"
#include "util.h"
#include "auditoria.h"
#include "config.h"
#include "menu.h"

static int leer_linea_limitada(char *dest, size_t tam)
{
    size_t longitud = 0;
    int caracter;
    int desbordada = 0;

    while ((caracter = getchar()) != '\n' && caracter != EOF) {
        if (longitud + 1 < tam) {
            dest[longitud++] = (char)caracter;
        } else {
            desbordada = 1;
        }
    }

    dest[longitud] = '\0';
    return desbordada ? ERROR : OK;
}

static int parsear_dni(const char *texto, long *dni)
{
    const char *caracter;
    char *fin;

    if (texto == NULL || *texto == '\0') {
        return ERROR;
    }

    for (caracter = texto; *caracter != '\0'; caracter++) {
        if (*caracter < '0' || *caracter > '9') {
            return ERROR;
        }
    }

    errno = 0;
    *dni = strtol(texto, &fin, 10);
    return errno == ERANGE || *fin != '\0' ? ERROR : OK;
}

static int parsear_fecha_texto(const char *texto, t_fecha *fecha)
{
    size_t i;

    if (texto == NULL || strlen(texto) != 10 ||
        texto[2] != '/' || texto[5] != '/') {
        return ERROR;
    }

    for (i = 0; i < 10; i++) {
        if (i != 2 && i != 5 && (texto[i] < '0' || texto[i] > '9')) {
            return ERROR;
        }
    }

    fecha->dia = (texto[0] - '0') * 10 + texto[1] - '0';
    fecha->mes = (texto[3] - '0') * 10 + texto[4] - '0';
    fecha->anio = (texto[6] - '0') * 1000 + (texto[7] - '0') * 100 +
                  (texto[8] - '0') * 10 + texto[9] - '0';
    return OK;
}

/*============================================================================*/
/*            FUNCIONES A IMPLEMENTAR POR EL ALUMNO (version _ALU)             */
/*============================================================================*/

/******************************************************************************
Descripcion:    convierte una linea CSV de 10 campos en t_socio. Los campos
                con formato invalido quedan con valores que haran fallar la
                validacion correspondiente (dni = 0, fecha 0/0/0, sexo '?',
                etc.). El estado se fija en 'A'.
Parametros:     linea: linea CSV.
                reg: puntero a t_socio destino.
Retorno:        OK si la cantidad de campos es correcta, ERROR en caso
                contrario.
Observaciones:  Se pasa por puntero como parsear a leer_archivo_texto.
******************************************************************************/
int texto_a_socio_ALU(const char *linea, void *reg)
{
    t_socio *socio = (t_socio *)reg;
    char buffer[TAM_LINEA];
    char *campos[CAMPOS_SOCIO];
    int cant_campos;


    socio->dni = 0;
    socio->cuil[0] = '\0';
    socio->apellidos_nombres[0] = '\0';
    socio->fecha_nacimiento.dia = 0; socio->fecha_nacimiento.mes = 0; socio->fecha_nacimiento.anio = 0;
    socio->sexo = '?';
    socio->fecha_afiliacion.dia = 0; socio->fecha_afiliacion.mes = 0; socio->fecha_afiliacion.anio = 0;
    socio->categoria[0] = '\0';
    socio->fecha_ultima_cuota.dia = 0; socio->fecha_ultima_cuota.mes = 0; socio->fecha_ultima_cuota.anio = 0;
    socio->plan[0] = '\0';
    socio->email_tutor[0] = '\0';
    socio->estado = 'A';


    strncpy(buffer, linea, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';


    char *salto_linea = buffer;
    while (*salto_linea != '\0' && *salto_linea != '\n')
    {
        salto_linea++;
    }
    if(*salto_linea == '\n')
        *salto_linea = '\0';


    cant_campos = separar_campos_ALU(buffer, campos, CAMPOS_SOCIO);


    if (cant_campos != CAMPOS_SOCIO) {
        return ERROR;
    }



    // DNI
    if (parsear_dni(campos[0], &socio->dni) != OK) {
        socio->dni = 0;
    }

    // CUIL
    if (strlen(campos[1]) >= sizeof(socio->cuil)) {
        strcpy(socio->cuil, "!");
    } else {
        strcpy(socio->cuil, campos[1]);
    }

    // Apellidos y Nombres
    strncpy(socio->apellidos_nombres, campos[2], sizeof(socio->apellidos_nombres) - 1);
    socio->apellidos_nombres[sizeof(socio->apellidos_nombres) - 1] = '\0';

    // Fecha de Nacimiento
    if (parsear_fecha_texto(campos[3], &socio->fecha_nacimiento) != OK) {
        socio->fecha_nacimiento.dia = 0; socio->fecha_nacimiento.mes = 0; socio->fecha_nacimiento.anio = 0;
    }

    // Sexo
    if (campos[4][0] != '\0') {
        socio->sexo = campos[4][0];
    } else {
        socio->sexo = '?';
    }

    // Fecha de AfiliaciOn
    if (parsear_fecha_texto(campos[5], &socio->fecha_afiliacion) != OK) {
        socio->fecha_afiliacion.dia = 0; socio->fecha_afiliacion.mes = 0; socio->fecha_afiliacion.anio = 0;
    }

    // CategorIa
    strncpy(socio->categoria, campos[6], sizeof(socio->categoria) - 1);
    socio->categoria[sizeof(socio->categoria) - 1] = '\0';

    // Ultima cuota
    if (parsear_fecha_texto(campos[7], &socio->fecha_ultima_cuota) != OK) {
        socio->fecha_ultima_cuota.dia = 0; socio->fecha_ultima_cuota.mes = 0; socio->fecha_ultima_cuota.anio = 0;
    }

    // Plan
    strncpy(socio->plan, campos[8], sizeof(socio->plan) - 1);
    socio->plan[sizeof(socio->plan) - 1] = '\0';

    // Email tutor
    if (strlen(campos[9]) >= sizeof(socio->email_tutor)) {
        strcpy(socio->email_tutor, "!");
    } else {
        strcpy(socio->email_tutor, campos[9]);
    }

    return OK;
}


/******************************************************************************
Descripcion:    aplica las validaciones del punto 6.1 en orden (DNI, Sexo,
                CUIL, Apellidos y Nombres, Fecha de nacimiento, Fecha de
                afiliacion, Categoria, Fecha de ultima cuota, Plan, Email
                tutor). Normaliza apellidos y nombres. Se detiene en el primer
                error.
Parametros:     reg: puntero a t_socio a validar (se modifica al normalizar).
                ctx: contexto con indice, archivo binario, fecha de proceso e
                id_modificando.
                tipo_error: salida; nombre de la validacion que fallo.
Retorno:        OK si el socio es valido, ERROR en caso contrario.
Observaciones:  Si ctx->id_modificando == dni no se considera duplicado y se
                excluye ese DNI al verificar unicidad del email. Cumple con el
                puntero validar de leer_archivo_texto.
******************************************************************************/
int valida_socio_ALU(void *reg, const t_contexto *ctx, char *tipo_error)
{
    t_socio *socio = (t_socio *)reg;
    int edad_socio;
    t_reg_indice clave;
    char cuil_calculado[TAM_CUIL];

    // DNI
    if (socio->dni <= DNI_MIN || socio->dni >= DNI_MAX) {
        strcpy(tipo_error, "DNI");
        return ERROR;
    }

    if (socio->dni != ctx->id_modificando) {
        clave.id = socio->dni;
        clave.nro_reg = 0;
        if (indice_buscar_ALU(ctx->indice, &clave, sizeof(clave),compara_reg_indice_ALU) != NO_EXISTE)
        {
            strcpy(tipo_error, "DNI");
            return ERROR;
        }
    }

    // Sexo
    if (socio->sexo != 'H' && socio->sexo != 'M') {
        strcpy(tipo_error, "SEXO");
        return ERROR;
    }

    // CUIL
    calcular_cuil_ALU(socio->dni, socio->sexo, cuil_calculado);
    if (strcmp(socio->cuil, cuil_calculado) != 0) {
        strcpy(tipo_error, "CUIL");
        return ERROR;
    }

    // Apellidos y Nombres
    arreglar_apyn_ALU(socio->apellidos_nombres);
    if (strlen(socio->apellidos_nombres) == 0) {
        strcpy(tipo_error, "APELLIDOS_NOMBRES");
        return ERROR;
    }

    // Fecha de nacimiento
    if (!fecha_correcta(&socio->fecha_nacimiento)) {
        strcpy(tipo_error, "FECHA_NACIMIENTO");
        return ERROR;
    }
    // Edad
    edad_socio = edad(&socio->fecha_nacimiento, ctx->fecha_proceso);
    if (edad_socio < EDAD_MINIMA) {
        strcpy(tipo_error, "FECHA_NACIMIENTO");
        return ERROR;
    }

    // Fecha de afiliacion
    if (!fecha_correcta(&socio->fecha_afiliacion) || compara_fechas(&socio->fecha_afiliacion, &socio->fecha_nacimiento) <= 0 || compara_fechas(&socio->fecha_afiliacion, ctx->fecha_proceso) > 0) {
        strcpy(tipo_error, "FECHA_AFILIACION");
        return ERROR;
    }

    // Categoria
    if (strcmp(socio->categoria, CAT_MENOR) != 0 &&strcmp(socio->categoria, CAT_ADULTO) != 0) {
        strcpy(tipo_error, "CATEGORIA");
        return ERROR;
    }
    if ((edad_socio < EDAD_ADULTO && strcmp(socio->categoria, CAT_MENOR) != 0) ||
        (edad_socio >= EDAD_ADULTO && strcmp(socio->categoria, CAT_ADULTO) != 0)) {
        strcpy(tipo_error, "CATEGORIA");
        return ERROR;
    }

    // Fecha de ultima cuota
    if (!fecha_correcta(&socio->fecha_ultima_cuota) || compara_fechas(&socio->fecha_ultima_cuota, &socio->fecha_afiliacion) < 0 || compara_fechas(&socio->fecha_ultima_cuota, ctx->fecha_proceso) > 0) {
        strcpy(tipo_error, "FECHA_ULTIMA_CUOTA");
        return ERROR;
    }

    // Plan
    if (strcmp(socio->plan, "BASIC") != 0 &&
        strcmp(socio->plan, "PREMIUM") != 0 &&
        strcmp(socio->plan, "VIP") != 0 &&
        strcmp(socio->plan, "FAMILY") != 0) {

        strcpy(tipo_error, "PLAN");
        return ERROR;
    }

    // Email Tutor
    if (strcmp(socio->categoria, CAT_MENOR) == 0) {
        if (!email_valido_ALU(socio->email_tutor)) {
            strcpy(tipo_error, "EMAIL_TUTOR");
            return ERROR;
        }
    } else if (socio->email_tutor[0] != '\0') {
        strcpy(tipo_error, "EMAIL_TUTOR");
        return ERROR;
    }

    if (socio->email_tutor[0] != '\0' &&
        socio_email_existe(ctx->arch_bin, ctx->indice, socio->email_tutor,
                           ctx->id_modificando)) {
        strcpy(tipo_error, "EMAIL_TUTOR");
        return ERROR;
    }

    return OK;
}

/******************************************************************************
Descripcion:    lee el registro nro_reg del archivo binario de socios.
Parametros:     arch: archivo abierto en modo binario.
                nro_reg: numero de registro.
                s: socio destino.
Retorno:        OK si se pudo leer, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_leer_ALU(FILE *arch, unsigned nro_reg, t_socio *s)
{

    if (fseek(arch, nro_reg * sizeof(t_socio), SEEK_SET) != 0)
    {
        return ERROR;
    }
    if (fread(s, sizeof(t_socio), 1, arch) != 1)
    {
        return ERROR;
    }


    return OK;
}

/******************************************************************************
Descripcion:    escribe el socio en la posicion nro_reg del archivo binario y
                hace flush.
Parametros:     arch: archivo abierto en modo de escritura binaria.
                nro_reg: numero de registro.
                s: socio a grabar.
Retorno:        OK si se pudo grabar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_escribir_ALU(FILE *arch, unsigned nro_reg, const t_socio *s)
{
    if (fseek(arch, nro_reg * sizeof(t_socio), SEEK_SET) != 0)
    {
        return ERROR;
    }

    if (fwrite(s, sizeof(t_socio), 1, arch) != 1)
    {
        return ERROR;
    }
    fflush(arch);
    return OK;
}

/******************************************************************************
Descripcion:    aplica la regla del Anexo A.4: el socio esta al dia si la
                ultima cuota es del mes de la fecha de proceso, o del mes
                anterior cuando el dia de la fecha de proceso es <= 10.
Parametros:     ultima_cuota: fecha de ultima cuota paga.
                fecha_proceso: fecha de proceso.
Retorno:        1 si esta al dia, 0 en caso contrario.
Observaciones:  -
******************************************************************************/
int cuota_al_dia_ALU(const t_fecha *ultima_cuota, const t_fecha *fecha_proceso)
{
    int mes_anterior;
    int anio_mes_anterior;


    if (ultima_cuota->mes == fecha_proceso->mes && ultima_cuota->anio == fecha_proceso->anio) {
        return 1;
    }

    if (fecha_proceso->dia > DIA_LIMITE_CUOTA) {
        return 0;
    }

    mes_anterior = fecha_proceso->mes - 1;
    anio_mes_anterior = fecha_proceso->anio;

    if (mes_anterior == 0)
    {
        mes_anterior = 12;
        anio_mes_anterior--;
    }

    return ultima_cuota->mes == mes_anterior && ultima_cuota->anio == anio_mes_anterior;
}

/******************************************************************************
Descripcion:    opcion Alta: carga un socio por teclado, lo valida con
                valida_socio_ALU, lo agrega al final de socios.dat con estado
                'A' y lo inserta en el indice. Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si el socio fue agregado, ERROR en caso contrario.
Observaciones:  La categoria se determina automaticamente segun la edad.
******************************************************************************/
int socio_alta_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    t_socio socio;
    t_contexto ctx;
    t_reg_indice reg_indice;

    char dni_texto[TAM_LINEA];
    char tipo_error[TAM_TIPO_ERROR];
    char fecha_texto[32];
    long posicion_archivo;
    unsigned nro_reg;

    FILE * arch = fopen(path, "r+b");
    if (arch == NULL) {
        printf("Error: no se pudo abrir %s.\n", path);
        return ERROR;
    }

    socio.dni = 0;
    socio.cuil[0] = '\0';
    socio.apellidos_nombres[0] = '\0';
    socio.fecha_nacimiento.dia = 0;
    socio.fecha_nacimiento.mes = 0;
    socio.fecha_nacimiento.anio = 0;
    socio.sexo = '?';
    socio.fecha_afiliacion.dia = 0;
    socio.fecha_afiliacion.mes = 0;
    socio.fecha_afiliacion.anio = 0;
    socio.categoria[0] = '\0';
    socio.fecha_ultima_cuota.dia = 0;
    socio.fecha_ultima_cuota.mes = 0;
    socio.fecha_ultima_cuota.anio = 0;
    socio.plan[0] = '\0';
    socio.email_tutor[0] = '\0';
    socio.estado = 'A';

    printf("DNI del socio: ");
    if (leer_linea_limitada(dni_texto, sizeof(dni_texto)) != OK) {
        socio.dni = 0;
    } else if (parsear_dni(dni_texto, &socio.dni) != OK) {
        socio.dni = 0;
    }

    printf("CUIL (formato XX-NNNNNNNN-Z): ");
    if (leer_linea_limitada(socio.cuil, sizeof(socio.cuil)) != OK) {
        strcpy(socio.cuil, "!");
    }

    printf("Apellidos y nombres: ");
    leer_linea(socio.apellidos_nombres, sizeof(socio.apellidos_nombres));

    printf("Fecha de nacimiento (dd/mm/aaaa): ");
    if (leer_linea_limitada(fecha_texto, sizeof(fecha_texto)) != OK ||
        parsear_fecha_texto(fecha_texto, &socio.fecha_nacimiento) != OK) {
        socio.fecha_nacimiento.dia = 0;
        socio.fecha_nacimiento.mes = 0;
        socio.fecha_nacimiento.anio = 0;
    }

    printf("Sexo (H/M): ");
    if (scanf(" %c", &socio.sexo) != 1) {
        socio.sexo = '?';
    }
    limpiar_entrada();



    printf("Fecha de afiliacion (dd/mm/aaaa): ");
    if (leer_linea_limitada(fecha_texto, sizeof(fecha_texto)) != OK ||
        parsear_fecha_texto(fecha_texto, &socio.fecha_afiliacion) != OK) {
        socio.fecha_afiliacion.dia = 0;
        socio.fecha_afiliacion.mes = 0;
        socio.fecha_afiliacion.anio = 0;
    }

    printf("Categoria (MENOR/ADULTO): ");
    if (leer_linea_limitada(socio.categoria, sizeof(socio.categoria)) != OK) {
        strcpy(socio.categoria, "!");
    }

    printf("Fecha de ultima cuota (dd/mm/aaaa): ");
    if (leer_linea_limitada(fecha_texto, sizeof(fecha_texto)) != OK ||
        parsear_fecha_texto(fecha_texto, &socio.fecha_ultima_cuota) != OK)
    {
        socio.fecha_ultima_cuota.dia = 0;
        socio.fecha_ultima_cuota.mes = 0;
        socio.fecha_ultima_cuota.anio = 0;
    }
    printf("Plan en mayusculas (BASIC/PREMIUM/VIP/FAMILY): ");
    leer_linea(socio.plan, sizeof(socio.plan));


    if (strcmp(socio.categoria, CAT_MENOR) == 0) {
        printf("Email del tutor: ");
        if (leer_linea_limitada(socio.email_tutor, sizeof(socio.email_tutor)) != OK) {
            strcpy(socio.email_tutor, "!");
        }
    }


    ctx.indice = indice;
    ctx.arch_bin = arch;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = 0;

    if (valida_socio_ALU(&socio, &ctx, tipo_error) != OK) {
        if (auditoria_registrar_ALU(ARCH_ERRORES, dni_texto, tipo_error,fecha_proceso) != OK) {
            printf("Error: no se pudo registrar el rechazo en la auditoria.\n");
        }
        printf("Alta rechazada: %s.\n", tipo_error);
        fclose(arch);
        return ERROR;
    }

    fseek(arch, 0, SEEK_END);
    posicion_archivo = ftell(arch);
    nro_reg = (unsigned)(posicion_archivo / (long)sizeof(t_socio));

    if (socio_escribir_ALU(arch, nro_reg, &socio) != OK) {
        printf("Error: no se pudo guardar el socio en %s.\n", path);
        if (fclose(arch) != 0) {
            printf("Error: no se pudo cerrar %s.\n", path);
        }
        return ERROR;
    }

    reg_indice.id = socio.dni;

    reg_indice.nro_reg = nro_reg;

    if (indice_insertar_ALU(indice, &reg_indice, sizeof(reg_indice), compara_reg_indice_ALU) != OK)
    {
        socio.estado = 'B';
        if (socio_escribir_ALU(arch, nro_reg, &socio) != OK) {
            printf("Error: no se pudo deshacer el alta en %s.\n", path);
        }
        printf("Error: no se pudo actualizar el indice de socios.\n");

        fclose(arch);
        return ERROR;
    }


    fclose(arch);
    printf("Socio dado de alta correctamente: DNI %ld.\n", socio.dni);
    return OK;
}

/******************************************************************************
Descripcion:    opcion Modificacion: pide el DNI, lo busca en el indice (un
                socio dado de baja no se puede modificar), muestra los datos
                actuales, pide los nuevos (Enter conserva), valida y
                sobrescribe el registro. El DNI y el estado no se modifican.
                Si es rechazado lo audita.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se modifico, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socio_modificacion_manual_ALU(t_indice *indice, const char *path, const t_fecha *fecha_proceso)
{
    t_socio socio;
    t_contexto ctx;
    t_reg_indice clave;
    char entrada[128];
    char dni_texto[32];
    char tipo_error[TAM_TIPO_ERROR];
    char cuil[TAM_CUIL];
    char apellidos_nombres[TAM_APYN];
    char plan[TAM_PLAN];
    char email_tutor[TAM_EMAIL];
    unsigned nro_reg;
    int edad_socio;

    FILE *arch = fopen(path, "r+b");
    if (!arch ) {
        printf("Error no se pudo abrir %s.\n", path);
        return ERROR;
    }

    printf("DNI del socio a modificar: ");
    if (scanf("%ld", &clave.id) != 1) {
        clave.id = 0;
    }
    limpiar_entrada();
    clave.nro_reg = 0;

    if (indice_buscar_ALU(indice, &clave, sizeof(clave), compara_reg_indice_ALU) == NO_EXISTE) {
        printf("Error no existe un socio activo con ese DNI.\n");
        fclose(arch);
        return ERROR;
    }

    nro_reg = clave.nro_reg;

    if (socio_leer_ALU(arch, nro_reg, &socio) != OK) {
        printf("Error no se pudo leer el socio de %s.\n", path);
        fclose(arch);
        return ERROR;
    }

    printf("Deje el campo vacio y presione Enter para conservar el dato actual.\n");

    printf("CUIL actual %s. Nuevo CUIL: ", socio.cuil);
    if (leer_linea_limitada(cuil, sizeof(cuil)) != OK) {
        strcpy(cuil, "!");
    }
    if (cuil[0] != '\0') {
        strcpy(socio.cuil, cuil);
    }

    printf("Apellidos y nombres actuales %s. Nuevos apellidos y nombres: ",socio.apellidos_nombres);
    leer_linea(apellidos_nombres, sizeof(apellidos_nombres));
    if (apellidos_nombres[0] != '\0') {
        strcpy(socio.apellidos_nombres, apellidos_nombres);
    }

    printf("Fecha de nacimiento actual %02d/%02d/%04d. Nueva fecha (dd/mm/aaaa): ",socio.fecha_nacimiento.dia, socio.fecha_nacimiento.mes,socio.fecha_nacimiento.anio);
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] != '\0') {
        if (parsear_fecha_texto(entrada, &socio.fecha_nacimiento) != OK) {
            socio.fecha_nacimiento.dia = 0;
            socio.fecha_nacimiento.mes = 0;
            socio.fecha_nacimiento.anio = 0;
        }
    }

    printf("Sexo actual %c. Nuevo sexo (H/M): ", socio.sexo);
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] != '\0') {
        socio.sexo = AMAYUS(entrada[0]);
    }

    printf("Fecha de afiliacion actual %02d/%02d/%04d. Nueva fecha (dd/mm/aaaa): ",socio.fecha_afiliacion.dia, socio.fecha_afiliacion.mes,socio.fecha_afiliacion.anio);
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] != '\0') {
        if (parsear_fecha_texto(entrada, &socio.fecha_afiliacion) != OK) {
            socio.fecha_afiliacion.dia = 0;
            socio.fecha_afiliacion.mes = 0;
            socio.fecha_afiliacion.anio = 0;
        }
    }

    edad_socio = edad(&socio.fecha_nacimiento, fecha_proceso);
    if (edad_socio >= 0) {
        strcpy(socio.categoria, edad_socio < EDAD_ADULTO ? CAT_MENOR : CAT_ADULTO);
    }

    printf("Fecha de ultima cuota actual %02d/%02d/%04d. Nueva fecha (dd/mm/aaaa): ", socio.fecha_ultima_cuota.dia, socio.fecha_ultima_cuota.mes, socio.fecha_ultima_cuota.anio);
    leer_linea(entrada, sizeof(entrada));
    if (entrada[0] != '\0') {
        if (parsear_fecha_texto(entrada, &socio.fecha_ultima_cuota) != OK) {
            socio.fecha_ultima_cuota.dia = 0;
            socio.fecha_ultima_cuota.mes = 0;
            socio.fecha_ultima_cuota.anio = 0;
        }
    }

    printf("Plan actual %s. Nuevo plan (BASIC/PREMIUM/VIP/FAMILY): ", socio.plan);
    leer_linea(plan, sizeof(plan));
    if (plan[0] != '\0') {
        cadena_a_mayusculas_ALU(plan);
        strcpy(socio.plan, plan);
    }

    if (strcmp(socio.categoria, CAT_MENOR) == 0) {
        printf("Email del tutor actual %s. Nuevo email: ", socio.email_tutor);
        if (leer_linea_limitada(email_tutor, sizeof(email_tutor)) != OK) {
            strcpy(email_tutor, "!");
        }
        if (email_tutor[0] != '\0') {
            strcpy(socio.email_tutor, email_tutor);
        }
    } else {
        socio.email_tutor[0] = '\0';
    }

    ctx.indice = indice;
    ctx.arch_bin = arch;
    ctx.fecha_proceso = fecha_proceso;
    ctx.id_modificando = socio.dni;

    if (valida_socio_ALU(&socio, &ctx, tipo_error) != OK) {
        sprintf(dni_texto, "%ld", socio.dni);
        if (auditoria_registrar_ALU(ARCH_ERRORES, dni_texto, tipo_error,fecha_proceso) != OK) {
            printf("Error no se pudo registrar el rechazo en la auditoria.\n");
        }
        printf("Modificacion rechazada: %s.\n", tipo_error);
        fclose(arch);
        return ERROR;
    }

    if (socio_escribir_ALU(arch, nro_reg, &socio) != OK)
    {
        printf("Error no se pudo guardar el socio modificado en %s.\n", path);
        fclose(arch);
        return ERROR;
    }

    fclose(arch);
    printf("Socio modificado correctamente: DNI %ld.\n", socio.dni);
    return OK;
}

/******************************************************************************
Descripcion:    baja logica de socio: busca el DNI en el indice, pone estado
                'B' en el registro del archivo y lo elimina del indice.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se dio de baja, ERROR si no existe o ya estaba dado de
                baja.
Observaciones:  -
******************************************************************************/
int socio_baja_ALU(t_indice *indice, const char *path)
{
    long dni;
    t_reg_indice clave;
    t_socio socio;
    int posicion;


    printf("DNI del socio a dar de baja: ");
    if(scanf("%ld", &dni) != 1) return ERROR;
    limpiar_entrada();

    clave.id = dni;
    clave.nro_reg = 0;
    posicion = indice_buscar_ALU(indice, &clave, sizeof(clave),  compara_reg_indice_ALU);
    if (posicion == NO_EXISTE) {
        printf("Error no existe un socio activo con DNI %ld.\n", dni);
        return ERROR;
    }

    FILE *arch = fopen(path, "r+b");
    if (!arch) {
        printf("Error no se pudo abrir %s.\n", path);
        return ERROR;
    }

    if (socio_leer_ALU(arch, clave.nro_reg, &socio) != OK) {
        printf("Error no se pudo leer el socio de %s.\n", path);
        fclose(arch);
        return ERROR;
    }

    socio.estado = 'B';
    if (socio_escribir_ALU(arch, clave.nro_reg, &socio) != OK) {
        printf("Error: no se pudo dar de baja el socio en %s.\n", path);
        fclose(arch);
        return ERROR;
    }

    if (indice_eliminar_ALU(indice, &clave, sizeof(clave),compara_reg_indice_ALU) != OK)
    {
        socio.estado = 'A';
        if (socio_escribir_ALU(arch, clave.nro_reg, &socio) != OK)
        {
            printf("Error no se pudo restaurar el estado del socio en %s.\n",
                   path);
        }
        printf("Error no se pudo eliminar el socio del indice.\n");
        fclose(arch);
        return ERROR;
    }

    fclose(arch);
    printf("Socio DNI %ld dado de baja correctamente.\n", dni);
    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice (ordenado por DNI y solo con activos) y
                lista los socios leyendo cada registro del archivo.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
Retorno:        OK si se pudo abrir el archivo, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_listar_ALU(const t_indice *indice, const char *path)
{

    t_reg_indice *registros_indice;
    t_socio socio;


    FILE *arch = fopen(path, "rb");
    if (!arch) {
        printf("Error no se pudo abrir %s.\n", path);
        return ERROR;
    }

    registros_indice = (t_reg_indice *)indice->vindice;
    printf("%-9s %-13s %-30s %-10s %-1s %-10s %-6s %-10s %s %-6s %s\n",
           "DNI", "CUIL", "APELLIDOS Y NOMBRES", "NACIMIENTO", "S",
           "AFILIACION", "CATEG.", "ULT.CUOTA", "E", "PLAN", "EMAIL TUTOR");

    unsigned i= 0;
    while ( i < indice->cantidad_elementos_actual)
    {
        if (socio_leer_ALU(arch, registros_indice[i].nro_reg, &socio) != OK) {
            printf("Error no se pudo leer el socio DNI %ld de %s.\n", registros_indice[i].id, path);
            fclose(arch);
            return ERROR;
        }
        socio_mostrar(&socio);
        i++;
    }

    printf("\nTotal de socios activos: %u\n",indice->cantidad_elementos_actual);
    fclose(arch);
    return OK;
}

/******************************************************************************
Descripcion:    recorre el indice y exporta a un archivo binario los socios
                con plan PREMIUM o VIP y cuota al dia (el estado 'A' lo
                garantiza el indice). Sobrescribe el archivo de exportacion
                anterior.
Parametros:     indice: indice de socios.
                path: ruta de socios.dat.
                path_export: ruta de beta_testers.dat.
                fecha_proceso: fecha de proceso.
Retorno:        OK si se pudo exportar, ERROR en caso contrario.
Observaciones:  -
******************************************************************************/
int socios_exportar_ALU(const t_indice *indice, const char *path, const char *path_export,
                    const t_fecha *fecha_proceso)
{
    FILE *arch_socios;
    FILE *arch_export;

    const t_reg_indice *registros_indice;
    t_socio socio;
    unsigned exportados = 0;

    arch_socios = fopen(path, "rb");
    if (!arch_socios) {
        printf("Error no se pudo abrir %s.\n", path);
        return ERROR;
    }

    arch_export = fopen(path_export, "wb");
    if (!arch_export) {
        printf("Error no se pudo crear %s.\n", path_export);
        fclose(arch_socios);
        return ERROR;
    }

    registros_indice = (const t_reg_indice *)indice->vindice;

    unsigned i = 0;
    while(i < indice->cantidad_elementos_actual)
    {
        if (socio_leer_ALU(arch_socios, registros_indice[i].nro_reg, &socio) != OK)
        {
            printf("Error no se pudo leer el socio DNI %ld de %s.\n",registros_indice[i].id, path);
            fclose(arch_socios);
            fclose(arch_export);
            return ERROR;
        }

        if ((strcmp(socio.plan, "PREMIUM") == 0 || strcmp(socio.plan, "VIP") == 0) && cuota_al_dia_ALU(&socio.fecha_ultima_cuota, fecha_proceso))
        {
            if (fwrite(&socio, sizeof(t_socio), 1, arch_export) != 1) {
                printf("Error no se pudo escribir el socio en %s.\n", path_export);
                fclose(arch_socios);
                fclose(arch_export);
                return ERROR;
            }
            exportados++;
        }
        i++;
    }
    printf("Socios exportados (betatester.dat): %u\n", exportados);
    fclose(arch_socios);
    fclose(arch_export);
    return OK;
}

/******************************************************************************
Descripcion:    opcion 2 del menu: submenu para elegir entre alta y
                modificacion manual de socio.
Parametros:     ind_socios: indice de socios.
                fecha_proceso: fecha de proceso.
Retorno:        No posee.
Observaciones:  -
******************************************************************************/
void abm_socio_ALU(t_indice *indice, const t_fecha *fecha_proceso)
{
    static const char OPCIONES_ABM[][LENGTH_MENU] =
    {
        "AMV",
        "Alta de socio",
        "Modificar socio",
        "Volver"
    };
    char seleccion;

    seleccion = menu(OPCIONES_ABM, "ABM de socios");
    switch (seleccion) {
        case 'A':
            socio_alta_manual_ALU(indice, ARCH_SOCIOS_BIN, fecha_proceso);
            break;
        case 'M':
            socio_modificacion_manual_ALU(indice, ARCH_SOCIOS_BIN,fecha_proceso);
            break;
        case 'V':
            break;
    }
}
