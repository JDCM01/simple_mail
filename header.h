#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Muy importante para strncpy, etc.
#include<sys/stat.h>   // mkdir
#include<unistd.h>     // chdir
#include<dirent.h>     // opendir, readdir, closedir


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

/*Constantes
*-----------
*MAX_SIZE: tamaño maximo de las strings 240 caracteres
*PORT_SIZE: tamaño que tendran las cadenas que almacenen los puertos
*IPV4_SIZE: tamaño que tendran las cadenas que almacenen las direcciones ipv4
*IPV6_SIZE: tamaño que tendran las cadenas que almacenen las direcciones ipv6
*/

#define MAX_SIZE 240
#define NAMES_SIZE 20
#define IPV4_SIZE  16
#define IPV6_SIZE 46
#define PORT_SIZE 5

/*send_and_receive
*-----------------
*Funcion para enviar mensajes a un cliente y recibir su respuesta copiandola 
*a un array que despues puedo manipular
*
*Argumentos:
*client_fd: descriptor de archivo para el socket del cliente
*message: Mensaje que se le enviara al cliente 
*answer: Respuesta que proporcionara el cliente
*MESSAGE_LENGTH: longitud del array del mensaje a enviar
*ANSWER_LENGTH: Longitud del array de la respuesta por parte del cliente
*/
void send_and_receive(int client_fd, char message[], char answer[], char receiver[], size_t MESSAGE_LENGTH, size_t ANSWER_LENGTH);

/*Funciones para el manejo de las strings*/

/*receive_message
*----------------
*Función que se encargara de recibir un mensaje y darselo a color_format
*
*Argumentos:
*client_fd: descriptor de archivos del socket del cliente
*receiver: nombre del que esta recibiendo el mensaje
*/
void receive_message(int client_fd, char receiver[]);

/*receive_and_send
*Funcion para enviar mensajes a un cliente y recibir su respuesta copiandola 
*a un array que despues puedo manipular
*
*Argumentos:
*client_fd: descriptor de archivo para el socket del cliente
*incoming_message: Mensaje entrante por parte del servidor 
*MESSAGE_LENGTH: longitud del array del mensaje a enviar
*ANSWER_LENGTH: Longitud del array de la respuesta por parte del cliente
*/
void receive_and_send(int client_fd, char incoming_message[], char receiver[],size_t MESSAGE_LENGTH, size_t ANSWER_LENGTH);


/*
*color_format
*------------
*Función para darle un color diferente a los mensajes, dependiendo de quien los
*envie, si son mensajes del mismo cliente deben verse de color azul, si son
*mensajes enviados por un cliente distinto de color morado y si son 
*mensajes de parte del servidor "Server" deben verse de color amarillo
*validara quien lo envio con ayuda de compare_strings 
*dependiendo del emisor se dara un formato a la salida
*Notas:
*\033 le indica a la consola que debe cambiar el color de la cadena
*\x1B le indica a la consola que debe cambiar el color de la cadena
*[33m] el color sera amarillo
*[34m] el color sera azul
*[35m] el color sera magenta 
*[0m] resetea el color
*
*Argumentos:
*Message: el mensaje al cual se le dara formato para mostrarlo la estructura del mensaje sera:
*"nombre_emisor"':'' '"mensaje"'\0' 
*client_name: nombre del cliente 
*/
void color_format(char message[], char receiver[]);

/*eliminate_from_string
*----------------------
*Función para eliminar un fragmento de una string, va a recorrer el array string
*hasta que se de una de las posibles condiciones de parada:
*i alcance la ultima posición del array, el caracter actual de la cadena sea '\0'
*el caracter actual sea el mark_character
*despues de esto empezara a copiar caracter a caracter de string a piece
*hasta que se de una de las condiciones de parada:
*i alcance la ultima posición del vector, el caracter actual de  string sea '\0'
*
*Argumentos:
*-string: cadena de la cual se eliminara un fragmento
*-piece: array donde se guardaran los caracteres restantes
*-mark_character: caracter que marcara el punto desde el cual debere empezar a copiar
*-MAX_LENGTH: longitud maxima de string
*/
void eliminate_from_string(char string[], char piece[], char mark_character, size_t MAX_LENGTH);

/*
*extract_from_string
*-------------------
*Función para extraer solo un fragmento de una cadena
*recorrera cada posición del array hasta encontrar un caracter especifico
*o encuentre '\0' para seguridad, e ira copiando en piece cada caracter que encuentre
*
*argumentos:
*string: cadena a la cual extraerle un fragmento
*piece: array donde se guardara la string extraida
*goal_character: caracter que indicara el punto de parada del ciclo deseada
*/
void extract_from_string(const char string[], char piece[], char goal_character);

/*
*check_string
*---------------
*Función para verificar que el cliente no este usando un nombre de usuario ya registrado previamente 
*ira de caracter en caracter dentro del fichero de usuarios hasta encontrar un ';'
*una vez allí comparara lo que ha leido vs la cadena que ha proporcionado el cliente
*en caso de ser iguales retornara un 1, en caso contrario continuara leyendo caracteres
*
*Argumentos:
*string_to_validate: Nombre de usuario o contraseña proporcionado por el cliente 
*file_pointer: puntero al archivo donde se guardaran los nombres de los clientes registrados
*option: Para poder reusar la función se necesita saber si se comprobara contraseña o nombre de 
*usuario, por lo cual option indica si es 0 es es el nombre de usuario 
*y debe ir recorriendo todo el archivo
*si es 1 indica que se comprobara contraseña y solo debe ir hasta que encuentre salto de linea
*o EOF
*
*Retorna:
*1: en caso de que el nombre de usuario ya se encuentre registrado o la contraseña coincida
*0: en caso de que no se encuentre registrado
*/
int check_string(char string_to_validate[], FILE *file_pointer, int option);

/*
*copy_string
*-----------
*funcion para copiar los elementos de una string en otra
*
*argumentos:
*-from_this: string que se debe copiar
*-to_this: lugar donde alojare la copia
*-length: ultima posicion del array
*/
void copy_string(char from_this[], char to_this[], size_t LENGTH);

/*
*from_int_to_char
*----------------
*Función para traducir de entero a caracter, segun la tabla ascii
*
*argumentos:
*character: caracter a traducir
*
*Retorno:
*un caracter que puede ir desde la a hasta z, punto, coma, dos puntos, 
*punto y coma, caracter nulo, salto,
*de linea
*/
char from_int_to_char(int character);

/*
*concatenate_string
*------------------
*Función para agregar mas caracteres al final de una string
*va a recorrer la string hasta que encuentre '\0' o caracter nulo
*que es donde termina la  string y lo reemplazara con el primer caracter
*de la cadena que se agregara, se ira copiando caracter a caracter hasta que 
*se encuentre '\0'
*
*argumentos:
*-this_string: string base
*-plus_this: string que se agregara al final de this_string
*/
void concatenate_string(char this_string[], char plus_this[], size_t MAX_LENGTH);

/*
*compare_strings
*---------------
*Función que comparara todos los caracteres de dos strings, y las recorrea hasta 
*que suceda una de las condiciones de parada:
*-el caracter actual analizado sea '\0' en alguna de las dos cadenas
*-algun caracter sea distinto 
*-se alcance la longitud máxima de las cadenas(MAX_SIZE)
*
*Argumentos:
*string_a: La primera de las dos strings
*string_b: La segunda de las dos strings
*
*Retorna:
*1 si recorre ambas strings hasta encontrar '\0' en ambas cadenas
*0 en cualquier otro caso
*/
int compare_strings(char string_a[], char string_b[]);

/*
* string_length:
* --------------
* Calcula la longitud de una cadena de caracteres recorriendo el arreglo
* hasta encontrar el carácter nulo ('\0') o hasta alcanzar el tamaño máximo
* especificado.
*
* Este límite evita leer fuera del arreglo en caso de que la cadena no esté
* correctamente terminada en '\0'.
*
* Parámetros:
* - string: arreglo de caracteres (cadena)
* - max_length: número máximo de caracteres a inspeccionar
*
* Retorna:
* - La cantidad de caracteres recorridos antes de encontrar '\0'
*   o alcanzar max_length.
*/
int string_length(const char string[], size_t LENGTH);

/*
* get_string
* ----------
* Lee una línea de entrada desde el teclado usando getchar() y la almacena
* en el arreglo 'string'.
*
* La lectura se detiene cuando ocurre alguno de los siguientes casos:
* - Se alcanza el fin de archivo (EOF)
* - Se encuentra un salto de línea ('\n')
* - Se alcanza la longitud máxima permitida (max_length - 1)
*
* Al finalizar, se agrega el carácter nulo ('\0') para indicar el final
* de la cadena.
*
* Parámetros:
* - message: arreglo donde se almacenará la cadena
*/
void get_string(char message[]);

/*
*get_arguments
*-------------
*
*Esta funcion recorrera un arreglo de caracteres volcando cada uno de 
*los caracteres de argv en string hasta que se de una de las posibles
*condiciones de parada:
*-i alcance la penultima posicion del arreglo sobre el cual estoy volcando la informacion
*-la posicion i del arreglo argv sea EOF indicando un error o final del archivo
*
*
*argumentos:
*-string_length: longitud maxima o ultima posicion de la string en la cual estoy volcando la informacion
*-argv: array de uno de los argumentos pasados por consola
*-argument_length: longitud del array argv
*/
void get_arguments(char string[], char argv[], size_t argument_length);

/*Funciones para el manejo de archivos*/
/*login
*------
*Función para comparar una contraseña que entregue el usuario con la que se
*halle en el archivo, se llamara a la función check_string, si la 
*contraseña se encuentra en el archivo la función retornara un 1 y
*se le consedera acceso al usuario al chat
*si se equivoca mas de 3 veces en digitar la contraseña se le denegara el acceso
*
*Argumentos:
*client_fd: descriptor de archivos del socket del cliente
*-file_pointer: Apuntador al archivo users.txt donde se encuentran los usuarios y las contraseñas
*
*Retorno:
*0: Si se deniega el acceso
*1: Si se concede el acceso
*/
int login(int client_fd, FILE *file_pointer);
/*
*register_user
*-------------
*Función para registrar un nombre de usuario en el archivo users.txt
*se le pide una contraseña al usuario para posteriormente insertar al nuevo
*usuario en users.txt  
*
*Argumentos:
*client_fd: descriptor de archivo del socket del cliente
*user_name: nombre del usuario a registrar
*file_pointer: apuntador al archivo users.txt  
*/
void register_user(int client_fd, char user_name[], FILE *file_pointer);

/*
*print_from_file
*----------------
*esta funcion va recorrer el archivo hasta que se cumpla la condicion:
*-getc entregue EOF indicando un error o el final del archivo
*
*parametros:
*file_pointer: apuntador al archivo
*/
void print_from_file(FILE *file_pointer);

/*
*insert_into_file
*----------------
*esta funcion va a iterar poniendo caracteres en el archivo hasta que se cumpla una de las condiciones:
*-i alcance la ultima posicion del vector string
*-el caracter actual sea '\0' o caracter nulo
*-putc entregue EOF indicando un error o el final del archivo
*
*parametros:
*file_pointer: apuntador al archivo
*-string: vetor de caracteres a insertar en el archivo
*-max_length: tamaño maximo del vector
*/
void insert_into_file(FILE *file_pointer, const char string[]);

/*Funciones y estructuras para el manejo de listas*/

//Candado para mutex MUTual EXclusion
pthread_mutex_t lock; 

/*Estructura Client
*------------------
*componentes:
*name: nombre del cliente
*port: puerto en el cual se dara la conversación
*ipv4: dirección del cliente IPv4
*ipv6: dirección del cliente IPv6
*/
typedef struct Client{
    char name[NAMES_SIZE];
    int client_fd;
    struct sockaddr_in client_address;
    socklen_t addr_len;
}Client;

/*Estructura List
*------------------
*componentes:
*user: apuntador a una estructura Client
*next: apuntador a la misma estructura que guardara la dirección de un siguiente nodo
*/
typedef struct List{
    Client* user;
    struct List* next;
}List;

/*
*thread_args
*-----------
*estructura para poder enviarle los argumentos a add_client
*
*componentes:
*server_fd: descriptor de archivo para el socket del server
*Client: apuntador una estructura de tipo Client para pasar a login_register lo que necesita 
*
*/
typedef struct thread_args{
    int server_fd;
    Client* client;
}thread_args;

/*
*threads_list
*------------
*estructura para hacer una lista de hilos 
*
*componentes:
*thread: es un hilo literalmente
*next: apuntador al siguiente hilo de la lista
*/
typedef struct threads_list{
    pthread_t thread;
    struct threads_list* next;
}threads_list;

/*
*add_thread
*----------
*Función para agregar hilos a la lista va a recibir un apuntador 
*a una estructura thread_list, va a apartar memoria para el nuevo
*nodo de la lista y el componente next de este nuevi hilo
*sera la lista, la lista pasara a ser igual a el nuevo nodo
*
*Argumentos:
*list: la lista de hilos actual
*/
void add_thread(threads_list** list);

/*
*show_list
*---------
*función que recorrera una lista compuesta de nodos que seran estructuras Client
*mostrando el nombre de cada cliente conectado
*hasta que se cumpla la condición de parada que el nodo actual sea igual a NULL osea que no halla nodo 
*
*Argumentos:
*stack: lista de clientes conectados al servidor
*/
void show_list(List* stack);

/*
*add_client
*----------
*Función para agregar clientes a la lista
*
*argumentos:
*list: lista actual de clientes conectados
*/
List* add_client(List** stack);

/*
*close_sockets
*-------------
*función que recorrera una lista compuesta de nodos que seran estructuras Client
*cerrando los sockets y liberando los espacios 
*en memoria hasta que se cumpla la condición de parada: que el nodo actual
*sea igual a NULL osea que no halla nodo 
*
*Argumentos:
*stack: lista de clientes conectados al servidor
*/
void close_sockets(List** stack);

/*
*register_login
*--------------
*Función para guardar espacio en memoria para un cliente y agregarlo a la lista 
*de usuarios conectados, hay tres casos posibles a la hora de agregarlo
*Caso 1, no hay nadie en la lista, en este caso stack sera igual al nuevo nodo creado
*Caso 2, ya hay gente en la lista, por lo cual el componente next de el nodo recien creado
*sera igual al primer nodo de la lista y este nodo recien creado
*sera el nuevo primer elemento de la lista
*
*Argumentos:
*server_fd: descriptor de archivo
*stack: Lista de clientes conectados
*///int server_fd, List** stack
void* register_login(void* args);


#endif

