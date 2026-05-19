#include"header.h"

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
void send_and_receive(int client_fd, char message[], char answer[], char receiver[], size_t MESSAGE_LENGTH, size_t ANSWER_LENGTH){
    char recv[ANSWER_LENGTH];
    memset(recv, 0, ANSWER_LENGTH);

    char message_to_send[MESSAGE_LENGTH];
    memset(message_to_send, 0, MESSAGE_LENGTH);

    copy_string(receiver, message_to_send, MESSAGE_LENGTH);
    concatenate_string(message_to_send, ": ", MESSAGE_LENGTH);
    concatenate_string(message_to_send, message, MESSAGE_LENGTH);

    // se envia un mensaje usando el descriptor del cliente osea lo que retorna el accept
    // Usamos string_length para enviar solo los caracteres necesarios (+1 para el '\0')
    write(client_fd, message_to_send, strlen(message_to_send));
    
    // Se lee lo que el cliente mande
    // read devuelve la cantidad de bytes leídos
    int bytes_leidos = read(client_fd, recv, ANSWER_LENGTH-1);
    if (bytes_leidos > 0) {
        recv[bytes_leidos] = '\0'; // Aseguramos que la cadena termine en nulo
        copy_string(recv, answer, ANSWER_LENGTH);
        color_format(answer, receiver);
    }
}

/*login
*------
*Función para comparar una contraseña que entregue el usuario con la que se
*halle en el archivo, se llamara a la función check_string, si la 
*contraseña se encuentra en el archivo la funcion terminara dandole acceso al cliente
*
*Argumentos:
*client_fd: descriptor de archivos del socket del cliente
*-file_pointer: Apuntador al archivo users.txt donde se encuentran los usuarios y las contraseñas
*/
void login(int client_fd, FILE *file_pointer){
    int result = 0;
    char answer[MAX_SIZE];
    char message[] = "Para poder continuar debera digitar su contraseña: \0";
    char wrong_password[] ="Contraseña incorrecta por favor digite una distinta\0";
    char access[MAX_SIZE] = "Server: Acceso garantizado\0";
    char blocking[MAX_SIZE] = "Server: Acceso denegado intentalo otra vez\0";
    while(result == 0){
        send_and_receive(client_fd, message, answer, "Server\0", MAX_SIZE, NAMES_SIZE);
        if(check_string(answer, file_pointer, 1) == 1){ 
            write(client_fd, access, string_length(access, MAX_SIZE));
            result = 1;
        }
        else{
            write(client_fd, blocking, string_length(blocking, MAX_SIZE));
            copy_string(wrong_password, message, string_length(wrong_password, MAX_SIZE));
        }    
    }
}
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
void register_user(int client_fd, char user_name[],FILE *file_pointer){
    char answer[MAX_SIZE];
    char user_to_register[MAX_SIZE];
    char ack[MAX_SIZE];
    copy_string(user_name, user_to_register, 100);
    char message[MAX_SIZE] = "Bienvenido al chat para poder registrarse debe digitar una contraseña\0";
    send_and_receive(client_fd, message, answer, "Server\0", MAX_SIZE, MAX_SIZE);  
    //char password[MAX_SIZE];
    //eliminate_from_string(answer, password, ' ', MAX_SIZE);  
    insert_into_file(file_pointer, "\n");
    concatenate_string(user_to_register, ";", MAX_SIZE);
    concatenate_string(user_to_register, answer, MAX_SIZE);
    //concatenate_string(user_to_register, ";", MAX_SIZE);
    concatenate_string(user_to_register, "\n\0", MAX_SIZE);
    insert_into_file(file_pointer, user_to_register);
    write(client_fd, "Server: Acceso garantizado\0", string_length("Server: Acceso garantizado\0", MAX_SIZE));
}


/*
*print_from_file
*----------------
*esta funcion va recorrer el archivo hasta que se cumpla la condicion:
*-getc entregue EOF indicando un error o el final del archivo
*
*parametros:
*file_pointer: apuntador al archivo
*/
void print_from_file(FILE *file_pointer){
    int c;
    printf(" ");
    while((c = getc(file_pointer)) != EOF){
        printf("%c", c);
    }    
}

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
void insert_into_file(FILE *file_pointer, const char string[]){
    int i = 0;
    while(i < MAX_SIZE - 1 && string[i] != '\0' && (putc(string[i], file_pointer) != EOF)){
        i++;
    }
}

