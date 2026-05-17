
#include"header.h"

/*receive_message
*----------------
*Función que se encargara de recibir un mensaje y darselo a color_format
*
*Argumentos:
*client_fd: descriptor de archivos del socket del cliente
*receiver: nombre del que esta recibiendo el mensaje
*/
void receive_message(int client_fd, char receiver[]){
    char recv[MAX_SIZE];
    char emissary[NAMES_SIZE];
    int bytes_leidos = read(client_fd, recv, sizeof(recv));
    if (bytes_leidos > 0) {
        recv[bytes_leidos] = '\0'; // Aseguramos que la cadena termine en nulo
        extract_from_string(recv, emissary, ':');
        color_format(recv, receiver);
    }
}

/*receive_and send
*-----------------
*Funcion para enviar mensajes a un cliente y recibir su respuesta copiandola 
*a un array que despues puedo manipular
*
*Argumentos:
*client_fd: descriptor de archivo para el socket del cliente
*incoming_message: Mensaje entrante por parte del servidor 
*MESSAGE_LENGTH: longitud del array del mensaje a enviar
*ANSWER_LENGTH: Longitud del array de la respuesta por parte del cliente
*/
void receive_and_send(int client_fd, char incoming_message[], char receiver[],size_t MESSAGE_LENGTH, size_t ANSWER_LENGTH){
    char recv[MESSAGE_LENGTH];
    char answer[ANSWER_LENGTH];
    char message_to_send[MESSAGE_LENGTH];

    // Se lee lo que el cliente mande
    // read devuelve la cantidad de bytes leídos    
    int bytes_leidos = read(client_fd, recv, sizeof(recv));
    if (bytes_leidos > 0) {
        recv[bytes_leidos] = '\0'; // Aseguramos que la cadena termine en nulo
        copy_string(recv, incoming_message, string_length(recv, MESSAGE_LENGTH));
        color_format(incoming_message, receiver);
    }

    printf("\nMensaje: ");
    get_string(answer);
    copy_string(receiver, message_to_send, string_length(receiver, MESSAGE_LENGTH));
    concatenate_string(message_to_send, ": ", MESSAGE_LENGTH);
    concatenate_string(message_to_send, answer, MESSAGE_LENGTH);

    // se recibe un mensaje usando el descriptor del cliente osea lo que retorna el accept
    // Usamos string_length para enviar solo los caracteres necesarios (+1 para el '\0')
    write(client_fd, message_to_send, string_length(message_to_send, MESSAGE_LENGTH));
}

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
void color_format(char message[], char receiver[]){
    char emissary[NAMES_SIZE];
    extract_from_string(message, emissary, ':');
    if(compare_strings(emissary, "Server\0") == 1){
        printf("\n\x1B[33m %s\x1B[0m", message);
    }
    else if(compare_strings(receiver, emissary) == 1){
        printf("\n\x1B[34m %s\x1B[0m", message);
    }
    else if(compare_strings(emissary, receiver) != 1){
        printf("\n\x1B[35m %s\x1B[0m", message);

    }
}

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
void eliminate_from_string(char string[], char piece[], char mark_character, size_t MAX_LENGTH){
    int i = 0;
    int j = 0;
    while(i < MAX_SIZE - 1 && string[i] != '\0' && string[i] != mark_character){
        i++;
    }
    i++;
    while(i < MAX_SIZE - 1 && string[i] != '\0'){
        piece[j] = string[i];
        j++;
        i++;
    }
    piece[j] = '\0';
}

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
void extract_from_string(const char string[], char piece[], char goal_character){
    int i = 0;
    while(i < MAX_SIZE - 1 && string[i] != '\0' && string[i] != goal_character){
        piece[i] = string[i];
        i++;
    }
    piece[i] = '\0';
}

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

int check_string(char string_to_validate[], FILE *file_pointer, int option){
    char actual_string[MAX_SIZE];
    int c, comparison_result;
    int i = 0;
    if(option == 0){
        while((c = getc(file_pointer)) != EOF){
            if(c == ';'){
                actual_string[i] = '\0';
                comparison_result = compare_strings(actual_string, string_to_validate);
                if(comparison_result == 1){
                    return 1;
                }
                i = 0;
            }
            else{
                if(c != '\n'){
                    actual_string[i] = c;    
                    i++;
                }
                
            }
        }
        return 0;  
    }
    else{
        while((c = getc(file_pointer)) != EOF && c != ';'){
            if(c != '\n' && c != ';'){
                actual_string[i] = c;
                i++;
            }
            
        }
        actual_string[i] = '\0';
        comparison_result = compare_strings(actual_string, string_to_validate);
        if(comparison_result == 1){
            return 1;
        }
        else{
            return 0;
        }  
    }
}


/*
*copy_string
*-----------
*funcion para copiar los elementos de una string en otra
*
*argumentos:
*-from_this: string que se debe copiar
*-to_this: lugar donde alojare la copia
*-length: ultima posicion del array
**/
void copy_string(char from_this[], char to_this[], size_t LENGTH){
    int i = 0;
    if(LENGTH == 0) return;
    while(i < LENGTH && from_this[i] != '\0'){
        to_this[i] = from_this[i];    
        i++;
    }
    to_this[i] = '\0'; 
}

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
char from_int_to_char(int character){
    int i;
    /*65-89 A-Z
    , = 44
    . = 46
    : = 58
    ; = 59
    \n = 10
    \0 = 0*/
    char *letters[] = {"a", "b", "c", "d", "f", "g", "h", "i", "j", 
                "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "\0"};
    if(character >= 65 && character <= 89){
        i = character - 65;
        return *letters[i];
    }else if(character == 44){
        return ',';
    }else if(character == 46){
        return '.';
    }else if(character == 58){
        return ':';
    }else if(character == 59){
        return ';';
    }else if(character == 10){
        return '\n';
    }else if(character == 0){
        return '\0';
    }
}

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
void concatenate_string(char this_string[], char plus_this[], size_t MAX_LENGTH){
    int i = 0;
    int j = 0;
    while(i < MAX_LENGTH -1 && this_string[i] != '\0'){    
        i++;
    }
    while(i < MAX_LENGTH -1 && plus_this[j] != '\0'){
        this_string[i] = plus_this[j];
        j++;
        i++;
    }
    this_string[i] = '\0';
}

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
int compare_strings(char string_a[], char string_b[]){
    int i = 0;
    while(i < MAX_SIZE && (string_a[i] != '\0') && (string_b[i] != '\0') ){
        if(string_a[i] != string_b[i]){
            return 0;
        }
        i++;
    }
    if((string_a[i] == string_b[i]) && (string_a[i] == '\0')){
        return 1;
    }else{
        return 0;
    }
}

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

int string_length(const char string[], size_t LENGTH){
    int i = 0;
    while(i < LENGTH - 1 && string[i] != '\0'){
        i += 1;
    }
    return i;
}

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
void get_string(char message[]){
    int character;
    int i = 0;
    while(i< MAX_SIZE - 1 && (character=getchar()) != EOF && character != '\n'){
        message[i] = character;
        i++;
    }
    message[i] = '\0';
}

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
void get_arguments(char string[], char argv[], size_t argument_length){
    int i = 0;
    int c;
    while(i < argument_length - 1 && argv[i] != EOF){//&& (argv += i) != EOF
        string[i] = argv[i];
        i++;
    }
    string[i] = '\0';
}