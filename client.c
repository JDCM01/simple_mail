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
        write(client_fd, password, sizeof(password));

        //recibiendo el permiso de continuar o la instrucción de enviar otra contraseña
        receive_message(client_fd, access, user_mail);
    }while(compare_strings(access, "Server: Acceso garantizado\0") != 1);
    color_format("Server: bienvenido al servicio de mensajeria", user_mail);
    
}