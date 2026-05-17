#include"header.h"

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
void add_thread(threads_list** list){
    threads_list* new_node = (threads_list*)malloc(sizeof(threads_list));
    new_node->next = *list;
    *list = new_node;
}

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
void show_list(List* stack){
    List* actual_node = (List*)malloc(sizeof(List));
    actual_node = stack;
    while(actual_node != NULL){
        printf("\nUsuario: %s", actual_node->user->name);
        actual_node = actual_node->next;
    }
}

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
void close_sockets(List** stack){
    if(stack == NULL && *stack == NULL){
        return;
    }

    while(stack != NULL && *stack != NULL){
        List* temp = *stack;
        printf("\nCerrando conexion con: %s", (*stack)->user->name);
        close(temp->user->client_fd);
        *stack = (*stack)->next;
        free(temp->user);
        free(temp);
    }
}

/*
*add_client
*----------
*Función para agregar clientes a la lista
*
*argumentos:
*list: lista actual de clientes conectados
*/
List* add_client(List** stack){
    List* new_node = (List*)malloc(sizeof(List));
    Client* new_client = (Client*)malloc(sizeof(Client));
    //creando una nueva estructura para el cliente que se conecte
    struct sockaddr_in client_address;
    new_node->user = new_client;
    new_node->user->client_address = client_address;
    printf("\ncliente, dirección de buffer: %p, tamaño: %zu\n", &new_node->user->client_address, sizeof(new_node->user->addr_len));
    new_node->next = *stack;
    *stack = new_node;
    return new_node; 
}

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
*/
void* register_login(void* args){
    thread_args* arguments = (thread_args*)args;
    int* access = malloc(sizeof(int));
    char message[] = "Server: Conexión establecida con: \0";
    char answer[NAMES_SIZE];
    char user_name[NAMES_SIZE];
    int read_bytes = read(arguments->client->client_fd, user_name, sizeof(user_name));
    if (read_bytes > 0) {
        user_name[read_bytes] = '\0'; // Aseguramos que la cadena termine en nulo
        copy_string(user_name, arguments->client->name, NAMES_SIZE);
        concatenate_string(message, user_name, MAX_SIZE);
        color_format(message, "Server\0");
    }

    pthread_mutex_lock(&lock); // se cierra el candado ya que vamos a modificar un archivo compartido
    FILE *file_pointer;
    file_pointer = fopen("users.txt","a+");
    if(check_string(user_name, file_pointer, 0) == 1){
        color_format("Server: Usuario encontrado se procede al login\0", "Server\0");
        *access = login(arguments->client->client_fd, file_pointer);
    }
    else{
        color_format("Server: No se encontro al usuario entonces se procede a guardarlo\0", "Server\0");
        register_user(arguments->client->client_fd, user_name, file_pointer);
        *access = 1;
    }

    fclose(file_pointer);
    pthread_mutex_unlock(&lock); // se abre el candado ya que salimos de la sección critica y debo darle la oportunidad 
    //a otro cliente de que se registre
    pthread_exit(access); // Devolvemos el puntero al resultado
}