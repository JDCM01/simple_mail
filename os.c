#include"header.h"

/*
*show_directory_content
*----------------------
*Función para revisar el contenido de un directorio
*ira recorriendo uno a uno las entradas del directorio
*y enviandole al cliente por medio del client_fd cada entrada
*
*Argumentos:
*-client_fd: descriptor de archivos del cliente
*-dir: data type para los directorios*/
void show_directory_content(int client_fd, DIR *dir){
    struct dirent *entry;
    char message[MAX_SIZE];
    memset(message, 0, sizeof(message));
    while((entry = readdir(dir)) != NULL){
        //write(client_fd, entry->d_name, string_length(entry->d_name, NAMES_SIZE));
        concatenate_string(message, entry->d_name, MAX_SIZE);
        concatenate_string(message, "\n", MAX_SIZE);
    }
    write(client_fd, message, string_length(message, MAX_SIZE));
    //write(client_fd, "end", sizeof("end"));
}

/*
*check_directory
*---------------
*Función que encargada de entrara a la carpeta de inbox
*o crearla en caso de que no exista, luego entrara
*al inbox del usuario especifico y mostrara la lista 
*de correos que tiene el usuario
*
*argumentos:
*-client_fd: descriptor de archivos del cliente
*client_mail: correo del cliente*/
void check_directory(int client_fd, char client_mail[]){

    //cambiamos la ubicación actual a la carpeta inbox
    if(chdir("inbox") == 1){
        perror("chdir fallo:");
    }

    //cambiamos la ubicación actual a la carpeta inbox
    if(chdir(client_mail) == 1){
        perror("chdir fallo: ");
    }

    //abriendo el directorio para poder revisar su contenido 
    DIR *dir = opendir(".");
    show_directory_content(client_fd, dir);

    //recibiendo el nombre del correo que el cliente quiere visualizar 
    //Recibiendo el comando del cliente (leer de forma robusta)
    char file_name[NAMES_SIZE];
    memset(file_name, 0, sizeof(file_name));
    int read_bytes = 0;
    
    //Leeyendo hasta recibir datos o error/EOF
    read_bytes = read(client_fd, file_name, sizeof(file_name) - 1);
    if (read_bytes <= 0) {
        if (read_bytes == 0) {
            printf("Cliente desconectado antes de enviar comando\n");
        } else {
            perror("Error leyendo comando del cliente");
        }
    } else {
        file_name[read_bytes] = '\0'; // Aseguramos que la cadena termine en nulo
    }

    //abriendo el archivo y leyendolo
    FILE* file_pointer = fopen(file_name, "r");
    char text[MAX_SIZE];
    memset(text, 0, sizeof(text));
    if (file_pointer != NULL) {
        extract_from_file(file_pointer, text);
        fclose(file_pointer);
        printf("\n%s", text);
        write(client_fd, text, string_length(text, MAX_SIZE));
    } else {
        printf("Error: no se pudo abrir el archivo %s\n", file_name);
        write(client_fd, "Error: archivo no encontrado", strlen("Error: archivo no encontrado"));
    }
    
    //cierro el directorio
    closedir(dir);
    
    //regresando al directorio anterior
    chdir("..");
    chdir("..");
}

/*
*store_message
*-------------
*Función para guardar un mensaje en el inbox del usuario deseado
*se le enviara por mensaje al cliente los correos registrados
*(osea los que esten presentes en el directorio inbox)
*se le pedira al cliente el correo de aquel usuario al que 
*quiera enviarle un mensaje, cuando se obtenga
*se le pedira al usuario el body del correo y se armara con la siguiente estructura
*SUBJECT: "cadena proporcionada por el cliente"
*BODY: "cadena proporcionada por el cliente"
*FROM: client_mail
*finalmente se almacenara esta cadena en un archivo de texto
*en el inbox del usuario designado bajo el nombre del subject mas un timestamp
*
*argumentos:
*-client_fd: descriptor de archivos del cliente
*-client_mail: mail del que va a enviar el correo
**/
#include <time.h> // REQUERIDO para el timestamp

void store_message(int client_fd, char client_mail[]) {
    int read_bytes;
    char receiver_mail[MAX_SIZE];
    char subject[MAX_SIZE];
    char body[MAX_SIZE];
    
    memset(receiver_mail, 0, sizeof(receiver_mail));
    memset(subject, 0, sizeof(subject));
    memset(body, 0, sizeof(body));

    //Mostrando usuarios disponibles 
    DIR *dir = opendir("inbox");
    if (dir == NULL) {
        perror("Error abriendo carpeta inbox");
        write(client_fd, "end", sizeof("end"));
        return;
    }
    show_directory_content(client_fd, dir); 

    //Leyendo destinatario
    read_bytes = read(client_fd, receiver_mail, sizeof(receiver_mail) - 1);
    if (read_bytes <= 0) return;
    receiver_mail[read_bytes] = '\0';

    //Leyendo asunto
    read_bytes = read(client_fd, subject, sizeof(subject) - 1);
    if (read_bytes <= 0) return;
    subject[read_bytes] = '\0';

    //Leyendo Cuerpo
    read_bytes = read(client_fd, body, sizeof(body) - 1);
    if (read_bytes <= 0) return;
    body[read_bytes] = '\0';

    //Construyendo el nombre del archivo 
    time_t t = time(NULL); 
    char file_name[MAX_SIZE * 2];
    snprintf(file_name, sizeof(file_name), "%s_%ld.txt", subject, (long)t);

    //Construyendo la ruta completa de destino
    size_t filepath_len = strlen("inbox/") + strlen(receiver_mail) + 1 + strlen(file_name) + 1;
    char *filepath = malloc(filepath_len);
    if (filepath == NULL) {
        perror("Error reservando memoria para la ruta del correo");
        return;
    }
    snprintf(filepath, filepath_len, "inbox/%s/%s", receiver_mail, file_name);

    //Intentando guardar el archivo
    FILE* file_pointer = fopen(filepath, "w");
    free(filepath);
    if (file_pointer == NULL) {
        perror("Error al crear el archivo del correo");
        return;
    }

    //Guardando 
    fprintf(file_pointer, "FROM: %s\n", client_mail);
    fprintf(file_pointer, "SUBJECT: %s\n", subject);
    fprintf(file_pointer, "BODY:\n%s\n", body);
    
    fclose(file_pointer);
}