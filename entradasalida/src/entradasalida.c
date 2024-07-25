#include "entradasalida.h"

//REVISAR TODOS LOS LOGS

int main(int argc, char *argv[]){
    log_entradasalida = log_create("./entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_TRACE);
    log_info(log_entradasalida, "INICIA EL MODULO DE ENTRADASALIDA");
    
    nombre_interfaz = malloc(100 * sizeof(char));
    ruta_archivo = malloc(100 * sizeof(char));

    //ruta_completa = "/home/utnso/so-deploy/tp-2024-1c-GoC/entradasalida/config/";
    ruta_completa = "/home/utnso/tp-2024-1c-GoC/entradasalida/config/";
    printf("Ingresa nombre de interfaz: ");
    scanf("%99s", nombre_interfaz);
    printf("\nIngresa el config: ");
    scanf("%99s", ruta_archivo);

    char* ruta_final = malloc(strlen(ruta_completa) + strlen(ruta_archivo) + 1);
    strcpy(ruta_final,ruta_completa);
    strcat(ruta_final,ruta_archivo);

    crear_interfaz(nombre_interfaz, ruta_final);

    generar_conexiones();
    inicializar_listas();
    
    pthread_t atiende_cliente_kernel;
    pthread_create(&atiende_cliente_kernel, NULL, (void *)recibirOpKernel, (void *) (intptr_t) conexion_entradasalida_kernel);
    pthread_join(atiende_cliente_kernel,NULL);
   
    log_info(log_entradasalida, "Finalizo conexion con servidores");
    finalizar_programa();
    return 0;
}

void crear_interfaz(char *nombre_interfaz, char *ruta_archivo){
    if (nombre_interfaz == NULL || ruta_archivo == NULL) {
        log_error(log_entradasalida, "El nombre de la interfaz o la ruta del archivo es NULL");
        return;
    }

    config_entradasalida = iniciar_config(ruta_archivo);

    if (config_entradasalida == NULL) {
        log_error(log_entradasalida, "No se pudo crear el config_entradasalida para el archivo %s", ruta_archivo);
        return;
    }

    tipo_interfaz_txt = config_get_string_value(config_entradasalida, "TIPO_INTERFAZ");

    if (tipo_interfaz_txt == NULL) {
        log_warning(log_entradasalida, "El archivo de configuración %s no tiene la clave TIPO_INTERFAZ", ruta_archivo);
        config_destroy(config_entradasalida);
        return;
    }
    if (strcmp(tipo_interfaz_txt, "GENERICA") == 0) {
        // Inicializar la interfaz Generica
        inicializar_interfaz_generica(config_entradasalida, nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "STDIN") == 0) {
        // Inicializar la interfaz Stdin
        inicializar_interfaz_stdin(config_entradasalida, nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "STDOUT") == 0) {
        // Inicializar la interfaz Stdout
        inicializar_interfaz_stdout(config_entradasalida,nombre_interfaz);
    } else if (strcmp(tipo_interfaz_txt, "DIALFS") == 0) {
        // Inicializar la interfaz DialFS
        inicializar_interfaz_dialfs(config_entradasalida, nombre_interfaz);
    } else {
        log_warning(log_entradasalida, "Tipo de interfaz desconocido: %s", tipo_interfaz_txt);
    }
}

void finalizar_programa(){
    free(nombre_interfaz);
    free(ruta_archivo);         

    log_destroy(log_entradasalida);
    config_destroy(config_entradasalida);
    dialfs_destroy(&fs);
}

void conexionRecMem(){
    pthread_t atiende_cliente_memoria; 
    pthread_create(&atiende_cliente_memoria, NULL, (void *)recibirOpMemoria, (void *) (intptr_t) conexion_entradasalida_memoria);
    pthread_detach(atiende_cliente_memoria);
}

void recibirOpKernel(int SOCKET_CLIENTE_KERNEL) {
    int noFinalizar = 0;
    while (noFinalizar != -1) {

        recibir_y_procesar_paquete(SOCKET_CLIENTE_KERNEL);

        if (list_is_empty(lista_operaciones)) {
            noFinalizar = -1;
            break;
        }

        operacionActual = *(op_code*)list_get(lista_operaciones, 0);

        bool operacionRealizada = false;

        switch (operacionActual) {
            case IO_GEN_SLEEP:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_sleep; 
                    pthread_create(&ejecutar_sleep, NULL, (void *)funcIoGenSleep, NULL);
                    pthread_detach(ejecutar_sleep);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_GEN_SLEEP_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_STDIN_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdRead; 
                    pthread_create(&ejecutar_StdRead, NULL, (void *)funcIoStdRead, NULL);
                    pthread_detach(ejecutar_StdRead);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_STDIN_READ_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_STDOUT_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_StdWrite; 
                    pthread_create(&ejecutar_StdWrite, NULL, (void *)funcIoStdWrite, NULL);
                    pthread_detach(ejecutar_StdWrite);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_STDOUT_WRITE_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_READ:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsRead; 
                    pthread_create(&ejecutar_FsRead, NULL, (void *)funcIoFsRead, NULL);
                    pthread_detach(ejecutar_FsRead);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_FS_READ_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_WRITE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsWrite; 
                    pthread_create(&ejecutar_FsWrite, NULL, (void *)funcIoFsWrite, NULL);
                    pthread_detach(ejecutar_FsWrite);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_FS_WRITE_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_CREATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsCreate; 
                    pthread_create(&ejecutar_FsCreate, NULL, (void *)funcIoFsCreate, NULL);
                    pthread_detach(ejecutar_FsCreate);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_FS_CREATE_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_DELETE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsDelete; 
                    pthread_create(&ejecutar_FsDelete, NULL, (void *)funcIoFsDelete, NULL);
                    pthread_detach(ejecutar_FsDelete);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_FS_DELETE_OK);
                    operacionRealizada = true;
                }
                break;
            case IO_FS_TRUNCATE:
                if (strcmp(nombreInterfazRecibido, nombre_interfaz) == 0 && es_operacion_compatible(tipoInterfaz, operacionActual)) {
                    pthread_t ejecutar_FsTruncate; 
                    pthread_create(&ejecutar_FsTruncate, NULL, (void *)funcIoFsTruncate, NULL);
                    pthread_detach(ejecutar_FsTruncate);
                    enviar_codop(SOCKET_CLIENTE_KERNEL, IO_FS_TRUNCATE_OK);
                    operacionRealizada = true;
                }
                break;
            default:
                log_warning(log_entradasalida, "Operacion no compatible");
                break;
        }

        if (operacionRealizada) {
            avanzar_a_siguiente_operacion();
        }
    }
}

void recibir_y_procesar_paquete(int socket_cliente) {
    int size;
    int desplazamiento = 0;
    void* buffer;

    op_code operacion = recibir_operacion(socket_cliente);
    list_add(lista_operaciones, &operacion);

    buffer = recibir_buffer(&size, socket_cliente);
    if (buffer == NULL) {
        printf("Error al recibir el paquete\n");
        return;
    }

    desplazamiento = 0;
    pidRecibido = leer_entero_uint32(buffer, &desplazamiento);
    list_add(lista_pids, &pidRecibido);

    switch (operacion) {
        case IO_GEN_SLEEP:
            nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
            unidadesRecibidas = leer_entero_uint32(buffer, &desplazamiento);
            list_add(lista_datos, nombreInterfazRecibido);
            list_add(lista_datos, &unidadesRecibidas);
            break;
        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:
            nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
            direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
            tamañoRecibido = leer_entero_uint32(buffer, &desplazamiento);
            list_add(lista_datos, nombreInterfazRecibido);
            list_add(lista_datos, &direccionRecibida);
            list_add(lista_datos, &tamañoRecibido);
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
            nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
            nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
            list_add(lista_datos, nombreInterfazRecibido);
            list_add(lista_datos, nombreArchivoRecibido);
            break;
        case IO_FS_TRUNCATE:
            nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
            nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
            tamañoArchivoRecibido = leer_entero_uint32(buffer, &desplazamiento);
            list_add(lista_datos, nombreInterfazRecibido);
            list_add(lista_datos, nombreArchivoRecibido);
            list_add(lista_datos, &tamañoArchivoRecibido);
            break;
        case IO_FS_WRITE:
        case IO_FS_READ:
            nombreInterfazRecibido = leer_string(buffer, &desplazamiento);
            nombreArchivoRecibido = leer_string(buffer, &desplazamiento);
            direccionRecibida = leer_entero_uint32(buffer, &desplazamiento);
            tamañoArchivoRecibido = leer_entero_uint32(buffer, &desplazamiento);
            RegistroPunteroArchivoRecibido = leer_entero_uint32(buffer, &desplazamiento);
            list_add(lista_datos, nombreInterfazRecibido);
            list_add(lista_datos, nombreArchivoRecibido);
            list_add(lista_datos, &direccionRecibida);
            list_add(lista_datos, &tamañoArchivoRecibido);
            list_add(lista_datos, &RegistroPunteroArchivoRecibido);
            break;
        default:
            break;
    }

    free(buffer);
}

//UNICA FUNCION QUE FALTA. LOS RECIBIR ETC
void recibirOpMemoria(int SOCKET_CLIENTE_MEMORIA){
    op_code operacion = recibir_operacion(SOCKET_CLIENTE_MEMORIA);
    char *mensaje;
    switch (operacion){
        case IO_STDOUT_WRITE:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_STDIN_READ:
            mensaje = recibir_string(SOCKET_CLIENTE_MEMORIA,log_entradasalida);
            log_info(log_entradasalida, "Valor escrito en memoria: %p", mensaje);
        break;
        case IO_FS_READ:
            log_info(log_entradasalida, "Mensaje escrito correctamente");
        break;
        case IO_FS_WRITE:
            mensaje = recibir_string(SOCKET_CLIENTE_MEMORIA,log_entradasalida);
            //terminando el write
            dialfs_write(&fs);
        break;
        default:
        break;
    }
}

void funcIoGenSleep(){
    log_info(log_entradasalida, "Generica: PID: <%d> - Gen_Sleep",pidRecibido);
    int unidades = unidadesRecibidas*tiempo_unidad_trabajo;
    log_info(log_entradasalida, "Eperando durante %d unidades",unidades);
    usleep(unidadesRecibidas*tiempo_unidad_trabajo);
    log_info(log_entradasalida, "Operacion completada");
}

void funcIoStdRead(){
    log_info(log_entradasalida, "Stdin: PID: <%d> - Leer.",pidRecibido);

    char *buffer = (char *)malloc(tamañoRecibido + 1);

    // Leer entrada del usuario
    if (fgets(buffer, tamañoRecibido + 1, stdin) == NULL) {
        log_error(log_entradasalida, "Error al leer desde STDIN");
        free(buffer);
        return;
    }

    // Validar que el tamaño de la entrada no exceda el tamaño especificado
    if (strlen(buffer) > tamañoRecibido) {
        log_warning(log_entradasalida, "El texto ingresado es mayor al tamaño permitido. Se truncará.");
        buffer[tamañoRecibido] = '\0';
    }

    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string=buffer;
    // Escribir el valor en la memoria. se encarga memoria yo solo le envio lo que escribio el usuario
    enviar_3_enteros_1_string(conexion_entradasalida_memoria, mensaje, IO_STDIN_READ);

    //recibo un OK de memoria
    conexionRecMem();
    free(buffer);
}

void funcIoStdWrite(){
    log_info(log_entradasalida, "Stdin: PID: <%d> - Escribir.",pidRecibido);

    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_STDOUT_WRITE);

    //Que memoria me pase lo leido y yo lo muestro en pantalla
    conexionRecMem();
}

void funcIoFsWrite(){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Leer archivo.",pidRecibido);
    //retardo
    usleep(tiempo_unidad_trabajo);
    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel

    t_3_enteros *mensaje = malloc(sizeof(t_3_enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    enviar_3_enteros(conexion_entradasalida_memoria,mensaje,IO_FS_WRITE);

    //Escribo el valor en el archivo
    conexionRecMem();
}

void funcIoFsRead(){
    log_info(log_entradasalida, "DialFS: PID: <%d> - Escribir Archivo.",pidRecibido);
    //retardo
    usleep(tiempo_unidad_trabajo);
    //char* textoLeido = dialfs_read(&fs);
    //envio a memoria un paquete con el RegistroTamaño y el RegistroDireccion recibidos de kernel
    t_string_3enteros *mensaje = malloc(sizeof(t_string_3enteros));
    mensaje->entero1 = direccionRecibida;
    mensaje->entero2 = pidRecibido;
    mensaje->entero3 = tamañoRecibido;
    mensaje->string = mensajeLeido;
    enviar_3_enteros_1_string(conexion_entradasalida_memoria,mensaje,IO_FS_READ);

    //recibo un OK de memoria o podemos hacer que se muestre lo que se escribio
    conexionRecMem();
}

void funcIoFsTruncate() {
    dialfs_truncar_archivo(&fs, nombreArchivoRecibido, tamañoArchivoRecibido);
}

void funcIoFsCreate() {
    int bloque = dialfs_crear_archivo(&fs, nombreArchivoRecibido, tamañoArchivoRecibido);

    if (bloque == -1) {
        return;
    }

    log_info(log_entradasalida, "Archivo creado con éxito: %s en bloque %d", nombreArchivoRecibido, bloque);
}

void funcIoFsDelete() {
    dialfs_destroy(&fs);
}

void generar_conexiones(){   
    if(tipoInterfaz != GENERICA_I){
        if (ip_memoria && puerto_memoria) {
            establecer_conexion_memoria(ip_memoria, puerto_memoria, config_entradasalida, log_entradasalida);
        }
        else {
            log_error(log_entradasalida, "Faltan configuraciones de memoria.");
        }
    }

    if (ip_kernel && puerto_kernel){
        establecer_conexion_kernel(ip_kernel, puerto_kernel, config_entradasalida, log_entradasalida);
    }
    else {
        log_error(log_entradasalida, "Faltan configuraciones de kernel.");
    }
}

void establecer_conexion_kernel(char *ip_kernel, char *puerto_kernel, t_config *config_entradasalida, t_log *loggs){
    log_trace(loggs, "Inicio como cliente Kernel");

    if ((conexion_entradasalida_kernel = crear_conexion(ip_kernel, puerto_kernel)) == -1)
    {
        log_error(loggs, "Error al conectar con Kernel. El servidor no está activo");
        return;
    }
    
    enviar_string(conexion_entradasalida_kernel, tipo_interfaz_txt, IDENTIFICACION);

}

void establecer_conexion_memoria(char *ip_memoria, char *puerto_memoria, t_config *config_entradasalida, t_log *loggs){
    log_trace(loggs, "Inicio como cliente Memoria");

    if ((conexion_entradasalida_memoria = crear_conexion(ip_memoria, puerto_memoria)) == -1)
    {
        log_error(loggs, "Error al conectar con Memoria. El servidor no está activo");
        return;
    }

}

void inicializar_interfaz_generica(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz genérica
    nombre = strdup(nombre);
    tipoInterfaz = GENERICA_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");

    //Ver si es necesario una funcion para mostrar interfaces, esto es para ver si las crea bien
    printf("Interfaz Generica creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: GENERICA\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
}

void inicializar_interfaz_stdin(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdin
    nombre = strdup(nombre);
    tipoInterfaz = STDIN_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    printf("Interfaz StdIn creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDIN\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
}

void inicializar_interfaz_stdout(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    tipoInterfaz = STDOUT_I;
    nombre = strdup(nombre);
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");

    printf("Interfaz StdOut creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n", puerto_memoria);
}

void inicializar_interfaz_dialfs(t_config *config_entradasalida, const char *nombre){
    // Inicializar atributos de la interfaz stdout
    nombre = strdup(nombre);
    tipoInterfaz = DIALFS_I;
    tiempo_unidad_trabajo = config_get_int_value(config_entradasalida, "TIEMPO_UNIDAD_TRABAJO");
    ip_kernel = config_get_string_value(config_entradasalida, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_entradasalida, "PUERTO_KERNEL");
    ip_memoria = config_get_string_value(config_entradasalida, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_entradasalida, "PUERTO_MEMORIA");
    path_base_dialfs = config_get_string_value(config_entradasalida, "PATH_BASE_DIALFS");
    block_size = config_get_int_value(config_entradasalida, "BLOCK_SIZE");
    block_count = config_get_int_value(config_entradasalida, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config_entradasalida, "RETRASO_COMPACTACION");

    printf("Interfaz DialFS creada:\n");
    printf("  Nombre: %s\n", nombre);
    printf("  Tipo de interfaz: STDOUT\n");
    printf("  Tiempo de unidad de trabajo: %d\n", tiempo_unidad_trabajo);
    printf("  IP Kernel: %s\n", ip_kernel);
    printf("  Puerto Kernel: %s\n", puerto_kernel);
    printf("  IP Memoria: %s\n", ip_memoria);
    printf("  Puerto Memoria: %s\n",puerto_memoria);
    printf("  Path Base: %s\n", path_base_dialfs);
    printf("  Block Size: %d\n", block_size);
    printf("  Block Count: %d\n",block_count);
    printf("  Retraso Compactacion: %d\n", retraso_compactacion);

    // Inicializar el sistema de archivos
    dialfs_init(&fs, block_size, block_count);
}

bool es_operacion_compatible(op_code tipo, op_code operacion){
    switch (tipo){
    case GENERICA_I:
        return operacion == IO_GEN_SLEEP;
    case STDIN_I:
        return operacion == IO_STDIN_READ;
    case STDOUT_I:
        return operacion == IO_STDOUT_WRITE;
    case DIALFS_I:
        return operacion == IO_FS_CREATE || operacion == IO_FS_DELETE || operacion == IO_FS_TRUNCATE || operacion == IO_FS_WRITE || operacion == IO_FS_READ;
    default:
        return false;
    }
}

void dialfs_init(DialFS *dialfs, int block_size, int block_count) {
    dialfs->num_blocks = block_count;
    dialfs->block_size = block_size;

    // Inicialización del bitmap de bloques
    dialfs->bitmap = (int *)malloc(block_count * sizeof(int));
    if (dialfs->bitmap == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el bitmap de bloques\n");
        exit(EXIT_FAILURE);
    }
    memset(dialfs->bitmap, 0, block_count * sizeof(int));

    // Inicialización de los bloques de datos
    dialfs->blocks = (Block *)malloc(block_count * sizeof(Block));
    if (dialfs->blocks == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para los bloques de datos\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < block_count; ++i) {
        dialfs->blocks[i].data = (uint8_t *)malloc(block_size * sizeof(uint8_t));
        if (dialfs->blocks[i].data == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria para los datos del bloque %d\n", i);
            exit(EXIT_FAILURE);
        }
        memset(dialfs->blocks[i].data, 0, block_size * sizeof(uint8_t)); // Inicializar los datos a 0
    }

    // Inicialización de la lista de archivos
    dialfs->archivos = list_create();
}

// Función para destruir el sistema de archivos DialFS y liberar memoria
void dialfs_destroy(DialFS *fs) {
    usleep(tiempo_unidad_trabajo);
    // Liberar la memoria de los archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *archivo = (Archivo *)list_get(fs->archivos, i);
        free(archivo->nombre_archivo);
        free(archivo);
    }
    list_destroy(fs->archivos);
    
    // Liberar los bloques y el bitmap
    free(fs->bitmap);
    for (int i = 0; i < fs->num_blocks; ++i) {
        free(fs->blocks[i].data);
    }
    free(fs->blocks);
    log_info(log_entradasalida, "DialFS: PID: <%d> - Eliminar Archivo.", pidRecibido);
}

// Función para reservar un bloque
int dialfs_allocate_block(DialFS *fs) {
    for (int i = 0; i < fs->num_blocks; i++) {
        if (fs->bitmap[i] == 0) {
            fs->bitmap[i] = 1; // Marcar como ocupado
            return i;
        }
    }
    return -1; // No hay bloques libres
}

// Función para liberar un bloque
void dialfs_free_block(DialFS *fs, int block_index) {
    if (block_index >= 0 && block_index < fs->num_blocks) {
        fs->bitmap[block_index] = 0; // Marcar como libre
    }
}

// Función para crear un archivo en DialFS
int dialfs_crear_archivo(DialFS *fs, const char *nombre_archivo, size_t tamaño) {
    usleep(tiempo_unidad_trabajo);
    //Es un sistema de archivo, debo crear el archivo en la direccion que viene del path + el nombre del archivo

    // Buscar un bloque libre para el archivo
    int bloque = dialfs_allocate_block(fs);
    if (bloque == -1) {
        fprintf(stderr, "No hay suficientes bloques libres para crear el archivo.\n");
        return -1;
    }

    // Crear la estructura para el archivo
    Archivo *nuevo_archivo = malloc(sizeof(Archivo));
    if (nuevo_archivo == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el nuevo archivo\n");
        return -1;
    }
    nuevo_archivo->nombre_archivo = strdup(nombre_archivo);
    nuevo_archivo->bloque_inicio = bloque;
    nuevo_archivo->tamaño = tamaño;

    // Agregar el archivo a la lista de archivos
    list_add(fs->archivos, nuevo_archivo);

    log_info(log_entradasalida, "DialFS: PID: <%d> - Crear Archivo: %s.", pidRecibido, nombre_archivo);
    return bloque;
}

// Función para redimensionar un archivo en DialFS
void dialfs_truncar_archivo(DialFS *fs, const char *nombre_archivo, size_t nuevo_size) {
    usleep(tiempo_unidad_trabajo);
    Archivo *archivo = NULL;

    // Buscar el archivo en la lista de archivos
    for (int i = 0; i < list_size(fs->archivos); ++i) {
        Archivo *a = (Archivo *)list_get(fs->archivos, i);
        if (strcmp(a->nombre_archivo, nombre_archivo) == 0) {
            archivo = a;
            break;
        }
    }

    if (archivo == NULL) {
        fprintf(stderr, "Archivo '%s' no encontrado.\n", nombre_archivo);
        return;
    }

    // Verificar si el tamaño del archivo necesita cambiar
    if (nuevo_size > archivo->tamaño) {
        // Necesita asignar más bloques si el nuevo tamaño es mayor
        size_t bytes_a_escribir = nuevo_size - archivo->tamaño;
        size_t bytes_por_bloque = fs->block_size;
        
        // Reasignar bloques si es necesario
        while (bytes_a_escribir > 0) {
            int bloque = dialfs_allocate_block(fs);
            if (bloque == -1) {
                fprintf(stderr, "No hay suficientes bloques libres para truncar el archivo '%s'.\n", nombre_archivo);
                return;
            }
            
            // Actualizar el archivo para usar el nuevo bloque
            // Suponiendo que tenemos una manera de agregar bloques adicionales a un archivo

            bytes_a_escribir -= bytes_por_bloque;
        }
        
        // Ajustar el tamaño del archivo en la lista
        archivo->tamaño = nuevo_size;
    } else if (nuevo_size < archivo->tamaño) {
        // Liberar bloques si el nuevo tamaño es menor
        size_t bytes_a_liberar = archivo->tamaño - nuevo_size;
        size_t bytes_por_bloque = fs->block_size;
        
        // Liberar bloques adicionales
        while (bytes_a_liberar > 0) {
            // Aquí podrías necesitar lógica adicional para encontrar y liberar bloques usados por el archivo
            int bloque = archivo->bloque_inicio; // Ejemplo de bloque a liberar
            dialfs_free_block(fs, bloque);
            
            bytes_a_liberar -= bytes_por_bloque;
        }
        
        // Ajustar el tamaño del archivo en la lista
        archivo->tamaño = nuevo_size;
    }

    log_info(log_entradasalida, "DialFS: PID: <%d> - Truncar Archivo: %s.", pidRecibido, nombre_archivo);
}

// Función para escribir en un archivo
void dialfs_write(DialFS *fs) {
    usleep(tiempo_unidad_trabajo);


}

// Función para leer un archivo
char* dialfs_read(DialFS *fs) {
    char* leido;
    usleep(tiempo_unidad_trabajo);



    return leido;
}

// Función para compactar archivos en DialFS
void dialfs_compactar_archivos(DialFS *fs) {
    log_info(log_entradasalida, "DialFS: PID: <%d> - Inicio Compactación.", pidRecibido);
    usleep(retraso_compactacion);
    int next_free_block = 0; // Siguiente bloque libre disponible

    // Recorrer todos los bloques del sistema de archivos
    for (int i = 0; i < fs->num_blocks; ++i) {
        if (fs->bitmap[i] == 1) {
            // si el bloque está ocupado, necesitamos copiarlo al próximo bloque libre disponible
            if (i != next_free_block) {
                // Copiar datos al próximo bloque libre
                memcpy(fs->blocks[next_free_block].data, fs->blocks[i].data, sizeof(Block));
                fs->bitmap[next_free_block] = 1; // Marcar el nuevo bloque como ocupado
                fs->bitmap[i] = 0; // Marcar el bloque original como libre
                
                // Actualizar el bloque de inicio en la lista de archivos
                for (int j = 0; j < list_size(fs->archivos); ++j) {
                    Archivo *archivo = (Archivo *)list_get(fs->archivos, j);
                    if (archivo->bloque_inicio == i) {
                        archivo->bloque_inicio = next_free_block;
                    }
                }
            }
            ++next_free_block; // Mover al siguiente bloque libre
        }
    }

    // Actualizar el número total de bloques ocupados
    fs->num_blocks = next_free_block;

    log_info(log_entradasalida, "DialFS: PID: <%d> - Fin Compactación.", pidRecibido);
}

void inicializar_listas() {
    lista_operaciones = list_create();
    lista_pids = list_create();
    lista_datos = list_create();
}

void avanzar_a_siguiente_operacion() {
    if (!list_is_empty(lista_operaciones)) {
        list_remove(lista_operaciones, 0);  // Remueve la operación actual
    }
}