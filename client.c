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
*list_registers
*--------------
*Función que estara recibiendo mensajes del servidor para 
*poder mostrar por pantalla a los clientes registrados en el servicio
*o los correos en el inbox del usuario
*
*argumentos:
*-client_fd: descriptor de archivos del socket del cliente
*/
void list_registers(int client_fd){
    char incomming_message[MAX_SIZE];
    do{
        receive_message(client_fd, incomming_message, " ");
    }
    while(compare_strings(incomming_message, "end\0")!=1);
}

/*
*listener_thread
*---------------
*Función para estar constantemente escuchando al servidor
*para saber si en algun momento se recibe un nuevo correo
*
*argumentos:
*-client_fd: descriptor de archivos del cliente
*/
void* listener_thread(void* args){
    int client_fd = (int*)args;
    char incomming_message[MAX_SIZE];
    while(1){
        receive_message(client_fd, incomming_message, "client\0");
    }
}

/*tener en cuenta que ahora la función de login se hara en bucle hasta que se entregue la contraseña correcta
asi que los receive and send deben estar en bucle en ese punto mientras que lo que retorne el servidor sea
"Server: Acceso denegado intentalo otra vez\0"*/

int main(void){
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Fallo en Winsock.\n");
        return 1;
    }
    #endif
    /*
    *Creando el socket
    *AF_INET: Indica que se usara IPv4. Si se quiere llegara a usar IPv6, sería AF_INET6.
    *SOCK_STREAM: Indica que se llevara a cabo 
    *una conexión TCP. Para UDP se usa SOCK_DGRAM
    *0: Protocolo por defecto osea IP
    */
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    int read_bytes;
    
    //Dandole identidad al cliente
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); // la dirección a la que se debe conectar
    address.sin_port = htons(8080);       // El puerto (htons convierte al orden de bytes de red)

    char user_mail[NAMES_SIZE];
    color_format("Server: Bienvenido al servicio de mensajeria para continuar digite su direccion de correo electrico: \0", " ");
    get_string(user_mail);

    //conectando al servidor
    if(connect(client_fd, (struct sockaddr *)&address, sizeof(address)) == -1){
        perror("Error al conectar con el servidor");
        exit(EXIT_FAILURE);
    }

    //Enviando el mail del usuario al servidor
    write(client_fd, user_mail, string_length(user_mail, NAMES_SIZE));

    //iniciando proceso de login o  register
    char incomming_message[MAX_SIZE];
    char password[NAMES_SIZE];
    char access[MAX_SIZE];
    do{
        //El servidor diciendole al cliente que debe digitar una contraseña
        receive_message(client_fd, incomming_message, user_mail);
        printf("\n");

        //capturando y enviando contraseña
        get_string(password);
        write(client_fd, password, string_length(password, NAMES_SIZE));

        //recibiendo el permiso de continuar o la instrucción de enviar otra contraseña
        receive_message(client_fd, access, user_mail);
    }while(compare_strings(access, "Server: Acceso garantizado\0") != 1);

    //Se concedio acceso y a continuación empieza el servicio de mensajeria
    color_format("Server: bienvenido al servicio de mensajeria", user_mail);

    char option_message[NAMES_SIZE] = {0};
    while(compare_strings(option_message, "quit\0") != 1){
        printf("\n\x1B[33mA continuación debera digitar una palabra para acceder a una de las funcionalidades\x1B[0m");
        printf("\n\x1B[33mquit: para desconectarse del servicio\x1B[0m");
        printf("\n\x1B[33msend: para construir y enviar un correo\x1B[0m");
        printf("\n\x1B[33mcheck: para revisar el inbox y ver correos\x1B[0m");
        printf("\n\x1B[33mEscriba la opción deseada por favor: \x1B[0m");
        get_string(option_message);
        write(client_fd, option_message, string_length(option_message, NAMES_SIZE));
        if(compare_strings("send\0", option_message) == 1){
            continue;
        }else if(compare_strings("check\0", option_message) == 1){
            list_registers(client_fd);
            printf("\n\x1B[33m A continuación digite el nombre del correo que desea abrir\x1B[0m");
            char file_name[MAX_SIZE] = {0};
            char text[MAX_SIZE];
            memset(text, 0, sizeof(text));
            get_string(file_name);
            write(client_fd, file_name, string_length(file_name, MAX_SIZE));
            read_bytes = read(client_fd, text, sizeof(text));
            printf("\n%s", text);
        }
        else if(compare_strings("quit\0", option_message) == 1){
            printf("\n\x1B[33m saliendo del servicio de mensajeria\x1B[0m");
        }
    }   
}