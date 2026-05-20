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
    while((entry = readdir(dir)) != NULL){
        write(client_fd, entry->d_name, string_length(entry->d_name, NAMES_SIZE));
    }
    write(client_fd, "end\0", string_length("end\0", MAX_SIZE));
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

    /*creando la carpeta donde se guardaran los registros del cliente en caso 
    de que no exista*/
    if(mkdir(client_mail, 0777) == -1){
        perror("mkdir fallo: ");
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
*en el inbox del usuario designado bajo el nombre del subject mas un timestamp*/
void store_message(int client_fd, char client_mail[]){

}