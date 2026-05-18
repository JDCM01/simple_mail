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
        Client* new_client = (Client*)malloc(sizeof(Client));
        struct sockaddr_in client_address;
        socklen_t addr_len = sizeof(struct sockaddr_in);

        int fd = accept(arguments->server_fd, (struct sockaddr*)&client_address, &addr_len);
        
        pthread_mutex_lock(&lock); // se cierra el candado ya que vamos a modificar un archivo compartido
            
        if (fd < 0) {
            perror("Error al aceptar");
            free(new_client);
            pthread_mutex_unlock(&lock);// se abre el candado para que no se quede bloqueado eternamente
            continue; 
        }
        else{
            new_client->client_fd = fd;   
            List* new_node = (List*)malloc(sizeof(List));
            new_node->user = new_client;
            new_node->next = *(arguments->client_list);
            *(arguments->client_list) = new_node;
            pthread_mutex_unlock(&lock); // se cierra el candado ya que vamos a modificar un archivo compartido    
        }
        
        /*Iniciando el proceso para acceder al servidor, se recibira el mail del usuario
        se comprobara si ya esta registrado, en cualquier caso se le enviara 
        un mensaje pidiendo una contraseña
        */
        register_login(arguments->server_fd, new_client->client_fd, new_client->mail);
        color_format("Server: Se llega al registro o logeo todo perfecto hasta aquí\0", "Server");
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

    if(listen(server_fd, 1) == -1){//listen retorna 0 cuando las conexiones son exitosas, -1 si hay errores 
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