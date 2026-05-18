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
        printf("\nUsuario: %s", actual_node->user->mail);
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
        printf("\nCerrando conexion con: %s", (*stack)->user->mail);
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
*Función para recibir el correo del cliente, se encargara de llamar 
*llamar a login o register dependiendo de:
*si el correo ya se encuentra dentro del archivo de usuarios llamara a login
*si el correo no se encuentra dentro del archivo de usuarios llamara a register
*
*Argumentos:
*server_fd: descriptor de archivo del socket del servidor
*client_fd: descriptor de archivo del cliente
*user_mail: correo electronico del usuario que desea logearse o registrarse
*/
void register_login(int server_fd, int client_fd, char user_mail[]){
    char message[MAX_SIZE] = "Server: Conexión establecida con: \0";
    char answer[NAMES_SIZE];
    char user[NAMES_SIZE];

    //pidiendole al cliente su correo
    int read_bytes = read(client_fd, user, sizeof(user));
    if (read_bytes > 0) {
        user[read_bytes] = '\0'; // Aseguramos que la cadena termine en nulo
        copy_string(user, user_mail, NAMES_SIZE);
        concatenate_string(message, user, MAX_SIZE);
        color_format(message, "Server\0");
    }

    pthread_mutex_lock(&lock); // se cierra el candado ya que vamos a modificar un archivo compartido
    FILE *file_pointer;
    file_pointer = fopen("users.txt","a+");
    if (file_pointer == NULL) {
        perror("Error al abrir archivo de usuarios");
        pthread_mutex_unlock(&lock);
        return;
    }

    if(check_string(user, file_pointer, 0) == 1){
        color_format("Server: Usuario encontrado se procede al login", "Server");
        login(client_fd, file_pointer);
    }
    else{
        color_format("Server: No se encontro al usuario entonces se procede a guardarlo", "Server");
        register_user(client_fd, user, file_pointer);
    }

    rewind(file_pointer);
    fclose(file_pointer);
    pthread_mutex_unlock(&lock); // se abre el candado ya que salimos de la sección critica y debo darle la oportunidad 
    //a otro cliente de que se registre
}