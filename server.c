#include"header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Muy importante para strncpy, etc.
#include<sys/stat.h>   // mkdir
#include<unistd.h>     // chdir
#include<dirent.h>     // opendir, readdir, closedir

#include"list_handler.c"
#include"file_handler.c"
#include"strings_handler.c"
#include"os.c"

#ifdef _WIN32
    /* --- CONFIGURACIÓN PARA WINDOWS --- */
    #include <winsock2.h>
    #include <ws2tcpip.h> // Necesario para funciones modernas de IP
    #pragma comment(lib, "ws2_32.lib")
    
    // En Windows, 'unistd.h' no existe de forma estándar
    // Definimos este alias para que 'close' funcione igual que en Linux
    #define close closesocket 
#else
    /* --- CONFIGURACIÓN PARA LINUX --- */
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif

#include <pthread.h>

/*
*accept_clients_args
*-------------------
*Estructura para poder pasarle los argumentos al hilo que se 
*encargara de supervisar las nuevas conexiones con los clientes
*
*componentes:
*-server_fd: descriptor de archivos del socket del servidor
*-client_list: Lista de clientes conectados al servidor*/
typedef struct accept_clients_args{
    int server_fd;
    List** client_list;
}accept_clients_args;

/*
*client_manager_arguments
*------------------------
*componentes:
*-client_fd: descriptor de archivos del socket del cliente
*-server_fd: descriptor de archivos del socket del cliente
*-client_list: lista de clientes conectados actualmente
*/
typedef struct client_manager_arguments{
    int client_fd;
    int server_fd;
    List** client_list;
}client_manager_arguments;

/*
*client_manager
*--------------
*Función que se encargara de todo el ciclo de vida del cliente
*hara el login, y mostrara el menu de mensajes, hara un read
*y dependiendo de el mensaje que envie el cliente almacenara en el inbox los 
*correos que el cliente cree y
*mostrara los correos que tenga en el inbox el cliente o se desconectara 
*
*posibles mensajes del cliente:
*-quit: para indicar desconexión
*-check: para revisar su inbox
*-send: para enviar un correo
*
*argumentos:
*-client_fd: descriptor de archivos del socket del cliente
*-server_fd: descriptor de archivos del socket del cliente
*-client_mail: mail del cliente para poder armar los correos
*este seria para hacer un from, quien envia
*/
void* client_manager(void* args){
    char client_mail[NAMES_SIZE];
    client_manager_arguments* arguments = (client_manager_arguments*)args;

    /*Iniciando el proceso para acceder al servidor, se recibira el mail del usuario
    se comprobara si ya esta registrado, en cualquier caso se le enviara 
    un mensaje pidiendo una contraseña
    */
    register_login(arguments->server_fd, arguments->client_fd, client_mail);
    fflush(stdout);   
    
    //añadiendo cliente a la lista
    pthread_mutex_lock(&lock); // se cierra el candado ya que vamos a modificar un recurso compartido
    Client* new_client = (Client*)malloc(sizeof(Client));
    new_client->client_fd = arguments->client_fd;   
    List* new_node = (List*)malloc(sizeof(List));
    new_node->user = new_client;
    new_node->next = *(arguments->client_list);
    *(arguments->client_list) = new_node;
    pthread_mutex_unlock(&lock); // se abre el candado ya que salio de la sección critica
    
    while(1){
        //Recibiendo el comando del cliente (leer de forma robusta)
        char command[NAMES_SIZE];
        memset(command, 0, sizeof(command));
        int read_bytes = 0;
        
        //Leeyendo hasta recibir datos o error/EOF
        read_bytes = read(arguments->client_fd, command, sizeof(command) - 1);
        if (read_bytes <= 0) {
            if (read_bytes == 0) {
                printf("Cliente desconectado antes de enviar comando\n");
            } else {
                perror("Error leyendo comando del cliente");
            }
            break;
        } else {
            command[read_bytes] = '\0'; // Aseguramos que la cadena termine en nulo
        }

        //ejecutando comando
        if(compare_strings(command, "quit\0") == 1){
            break;
        }else if(compare_strings(command, "check\0") == 1){
            check_directory(arguments->client_fd, client_mail);
        }else if(compare_strings(command, "send\0") == 1){
            printf("under constructio");
        }
    }

    //protegiendo la sección critica de la lista
    List* current = *(arguments->client_list);
    List* previous = NULL;
    int found = 0;

    // Recorriendo la lista para encontrar al nodo del cliente y eliminarlo
    while (current != NULL) {
        if (current->user->client_fd == arguments->client_fd) {
            if (previous == NULL) {
                // El nodo a eliminar es el primero de la lista
                *(arguments->client_list) = current->next;
            } else {
                // El nodo está en medio o al final
                previous->next = current->next;
            }
            
            // Liberando la memoria
            free(current->user); 
            free(current);
            found = 1;
            break;
        }
        previous = current;
        current = current->next;
    }

    //liberando el recurso
    pthread_mutex_unlock(&lock);

    // Cerrando el socket del cliente
    close(arguments->client_fd);

    // Liberando la estructura de argumentos 
    free(arguments);
    
    pthread_exit(NULL);
}

/*
*accept_clients
*--------------
*Función que estara corriendo en un hilo constantemente para poder aceptar clientes en cualquier
*momento, despues de ser aceptado el cliente pasara al proceso de login_register, cuando se 
*le de acceso al servicio se agregara a la lista de usuarios conectados y el hilo le dara 
*el control al main_thread a cada cliente
*
*Argumentos:
*-clients_list: Lista de clientes conectados actualmente para poder agregar de manera dinamica
*aquellos clientes que se vallan conectando al servidor
*-server_fd: Descriptor de archivos del server*/
void* accept_clients(void* args){
    accept_clients_args* arguments = (accept_clients_args*)args;
    while(1){
        //aceptando la conexion con un cliente
        struct sockaddr_in client_address;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        int fd = accept(arguments->server_fd, (struct sockaddr*)&client_address, &addr_len);
        
        if (fd < 0) {
            perror("Error al aceptar");
            continue; 
        }
        else{
            //Creando los argumentos para el hilo de client_manager
            client_manager_arguments* manager_args = (client_manager_arguments*)malloc(sizeof(client_manager_arguments));
            manager_args->client_fd = fd;
            manager_args->server_fd = arguments->server_fd; 
            manager_args->client_list = arguments->client_list;
        
            //Creando hilo para client_manager y desplegandolo
            pthread_t manager_thread;
            if (pthread_create(&manager_thread, NULL, client_manager, manager_args) != 0) {
                perror("Error al crear el hilo de aceptación");
                exit(EXIT_FAILURE);
            }

            //los recursos usados por manager_thread podran ser reclamados cuando el hilo termine
            pthread_detach(manager_thread);
        }
    }
}

int main(void){
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Fallo en Winsock.\n");
        return 1;
    }
    #endif
    
    /*creando la carpeta inbox, en caso de que no exista, se le conceden
    permisos de: lectura, escritura, ejecucion para propietario y otros usuarios*/ 
    if(mkdir("inbox", 0777) == -1){
        perror("mkdir fallo:");
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    //Si el puerto está ocupado por una instancia anterior que acabo de cerrar, déjame reusarlo de inmediato
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    //Dandole identidad al servidor
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las IPs disponibles
    address.sin_port = htons(8080);       // El puerto (htons convierte al orden de bytes de red)
    
    //haciendo bind: Cualquier dato que llegue a este puerto específico, dáselo a este programa
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 0) == -1){//listen retorna 0 cuando las conexiones son exitosas, -1 si hay errores 
        perror("Error al escuchar a los clientes");
        exit(EXIT_FAILURE);
    }

    //Creando los argumentos para la función accept_clients
    List* client_list = NULL;
    accept_clients_args* accept_args = (accept_clients_args*)malloc(sizeof(accept_clients_args));
    accept_args->client_list = &client_list;
    accept_args->server_fd = server_fd;

    //desplegando el hilo
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, accept_clients, accept_args) != 0) {
        perror("Error al crear el hilo de aceptación");
        exit(EXIT_FAILURE);
    }

    //esperando al bucle de aceptación
    pthread_join(accept_thread, NULL); 

    // Limpieza al salir (si es que el bucle terminara)
    free(accept_args);
    pthread_mutex_destroy(&lock);
}