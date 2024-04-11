#include <kernel.h>

int main(int argc, char* argv[]) {

    log_kernel = log_create("./kernel.log", "KERNEL", 1, LOG_LEVEL_TRACE);

    log_info(log_kernel, "INICIA EL MODULO DE KERNEL");

    config_kernel = iniciar_config("./config/kernel.config");
        
    puerto_escucha = config_get_string_value(config_kernel, "PUERTO_ESCUCHA");
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    ip_cpu = config_get_string_value(config_kernel, "IP_CPU");
    puerto_cpu_interrupt = config_get_string_value(config_kernel, "PUERTO_CPU_INTERRUPT");
    puerto_cpu_dispatch = config_get_string_value(config_kernel, "PUERTO_CPU_DISPATCH");

    algoritmo = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    if (strcmp(algoritmo, "FIFO") == 0)
    {
        algoritmo_planificacion = FIFO;
    }
    else if (strcmp(algoritmo, "RR") == 0)
    {
        algoritmo_planificacion = RR;
    }
    else if (strcmp(algoritmo, "VRR") == 0)
    {
        algoritmo_planificacion = VRR;
    }
    else
    {
        log_error(log_kernel, "El algoritmo no es valido");
    }
    quantum = config_get_string_value(config_kernel, "QUANTUM");

    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_int_value(config_kernel, "QUANTUM");

    recursos = config_get_string_value(config_kernel, "RECURSOS");
    instancias_recursos = config_get_string_value(config_kernel, "INSTANCIAS_RECURSOS");
    grado_multiprogramacion = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");

    log_info(log_kernel, "levanto la configuracion del kernel");

    iniciar_semaforos();

    establecer_conexion_cpu(ip_cpu, puerto_cpu_dispatch, config_kernel, log_kernel);
    
    establecer_conexion_memoria(ip_memoria, puerto_memoria, config_kernel, log_kernel);

    log_info(log_kernel, "Finalizo conexion con servidores");


    socket_servidor_kernel_dispatch = iniciar_servidor(puerto_escucha, log_kernel);

    log_info(log_kernel, "INICIO SERVIDOR");

    log_info(log_kernel, "Listo para recibir a EntradaSalida");

    socket_cliente_entradasalida = esperar_cliente(socket_servidor_kernel_dispatch);

    pthread_t atiende_cliente_entradasalida;
    pthread_create(&atiende_cliente_entradasalida, NULL, (void *)recibir_entradasalida, (void *) (intptr_t) socket_cliente_entradasalida);
    pthread_detach(atiende_cliente_entradasalida);
    
    iniciar_consola();


    log_info(log_kernel, "Finalizo conexion con cliente");//nunca finalizamos la conexion usar liberar_conexion


    return 0;
}

void iniciar_semaforos(){
    pthread_mutex_init(&mutex_cola_ready, NULL);
    pthread_mutex_init(&mutex_cola_new, NULL);

    sem_init(&sem_multiprogamacion, 0, grado_multiprogramacion);
    sem_init(&sem_listos_para_ready, 0, 0);
}

void recibir_entradasalida(int SOCKET_CLIENTE_ENTRADASALIDA){
    enviar_mensaje("recibido entradasalida", SOCKET_CLIENTE_ENTRADASALIDA); 
    //Se deben enviar la cantidad de unidades de trabakp necesarias, crear una nueva funcion

}

void establecer_conexion_cpu(char * ip_cpu, char* puerto_cpu_dispatch, t_config* config, t_log* loggs){

    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto CPU %s ", ip_cpu, puerto_cpu_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel = crear_conexion(ip_cpu, puerto_cpu_dispatch)) == -1){
        log_trace(loggs, "Error al conectar con CPU. El servidor no esta activo");

        exit(2);
    }

    //recibir_operacion(conexion_kernel);
    recibir_mensaje(conexion_kernel,loggs);
}

void establecer_conexion_memoria(char* ip_memoria, char* puerto_memoria_dispatch, t_config* config, t_log* loggs){


    log_trace(loggs, "Inicio como cliente");

    log_trace(loggs,"Lei la IP %s , el Puerto Memoria %s ", ip_memoria, puerto_memoria_dispatch);

    // Enviamos al servidor el valor de ip como mensaje si es que levanta el cliente
    if((conexion_kernel = crear_conexion(ip_memoria, puerto_memoria_dispatch)) == -1){
        log_trace(loggs, "Error al conectar con Memoria. El servidor no esta activo");

        exit(2);
    }

    //recibir_operacion(conexion_kernel);
    recibir_mensaje(conexion_kernel,loggs);
}

void iniciar_consola(){
    int eleccion;
    printf("Operaciones disponibles para realizar:\n");
    printf("1. Ejecutar Script de Operaciones\n");
    printf("2. Iniciar proceso\n");
    printf("3. Finalizar proceso\n");
    printf("4. Iniciar planificación\n");
    printf("5. Detener planificación\n");
    printf("6. Listar procesos por estado\n");

    printf("seleccione la opción que desee: ");
    scanf("%d", &eleccion);

    switch (eleccion)
    {
    case 1:
        ejecutar_script();
        break;
    case 2:
        iniciar_proceso();
        break;
    case 3:
        finalizar_proceso();
        break;
    case 4:
        iniciar_planificacion();
        break;
    case 5:
        detener_planificacion();
        break;
    case 6:
        listar_procesos_estado();
        break;
    default:
        printf("Opción no valida intente de nuevo!\n");
        iniciar_consola();
        break;
    }
}


void ejecutar_script(){

}

void iniciar_proceso(){
    char* path = malloc(30*sizeof(char));

    printf("Por favor ingrese el path: ");
    scanf("%s", path);

    //ACA HAY QUE AVISARLE A MEMORIA QUE SE CREA UN PROCESO DE ESE PATH, DEBERIA DEVOLVER ALGUNA INFO?

    //creamos PCB
    t_registros_cpu* registros = inicializar_registros();
    t_pcb* pcb_nuevo = malloc(sizeof(t_pcb));
    pcb_nuevo->estado = NEW;
    pcb_nuevo->pid = generador_pid++;
    pcb_nuevo->pc = 0;
    pcb_nuevo ->registros = registros;
    pthread_mutex_lock(&mutex_cola_new);
    list_add(cola_new, pcb_nuevo);
    pthread_mutex_unlock(&mutex_cola_new);
    sem_post(&sem_listos_para_ready);
}

void finalizar_proceso(){

}

void iniciar_planificacion(){

}

void detener_planificacion(){

}


void listar_procesos_estado(){

}

t_registros_cpu* inicializar_registros(){
    t_registros_cpu* registros = malloc(sizeof(t_registros_cpu));

    registros->AX = malloc(sizeof(uint8_t));
    registros->BX = malloc(sizeof(uint8_t));
    registros->CX = malloc(sizeof(uint8_t));
    registros->DX = malloc(sizeof(uint8_t));
    registros->EAX = malloc(sizeof(uint32_t));
    registros->EBX = malloc(sizeof(uint32_t));
    registros->ECX = malloc(sizeof(uint32_t));
    registros->EDX = malloc(sizeof(uint32_t));

    return registros;
}

void planificar_largo_plazo(){
     pthread_t hilo_ready;
    
    pthread_create(&hilo_ready, NULL, (void *)pcb_ready, log_kernel);

    pthread_detach(hilo_ready);
}

t_pcb* remover_pcb_de_lista(t_list *list, pthread_mutex_t *mutex)
{
    t_pcb* pcbDelProceso = malloc(sizeof(t_pcb));
    pthread_mutex_lock(mutex);
    pcbDelProceso = list_remove(list, 0);
    pthread_mutex_unlock(mutex);
    return pcbDelProceso;
}

void pcb_ready(){
    sem_wait(&sem_listos_para_ready);
    t_pcb* pcb = remover_pcb_de_lista(cola_new, &mutex_cola_new);
    sem_wait(&sem_multiprogamacion);
    pcb->estado = READY;
    pthread_mutex_lock(&mutex_cola_ready);
    list_add(cola_ready,pcb);
    pthread_mutex_unlock(&mutex_cola_ready);
    //sem_post(); para habilitar a que pase a ejecutar
}

t_pcb* elegir_pcb_segun_algoritmo(){
    t_pcb* pcb_ejecutar = malloc(sizeof(t_pcb));

    switch (algoritmo_planificacion)
    {
    case FIFO:
        pthread_mutex_lock(&mutex_cola_ready);
        pcb_ejecutar = list_remove(cola_ready,0);
        pthread_mutex_unlock(&mutex_cola_ready);
        break;
    case RR:
        break;
    case VRR:
        break;
    default:
        break;
    }
    return pcb_ejecutar;
}

