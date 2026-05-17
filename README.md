# Chat en C

Pequeño servicio de mensajeria cliente-servidor desarrollado en lenguaje C.

## Compilación


La manera para poder correrlo en linux es:
```bash
gcc server.c -o server -lpthread
gcc client.c -o client -lpthread
```

La manera para poder correrlo en windows es:

Para windows:
-En caso de que cygwin1.dll este instalado hacer lo mismo que en linux
-En caso contrario solo hay que instalar GCC en la terminal de WSL (Ubuntu)
sudo apt update && sudo apt install build-essential
```bash
gcc server.c -o server -lpthread
gcc client.c -o client -lpthread
```
-Sino hay ninguna de las dos rezar porque halla Dev-c++ o CODE::BLOCKS con MinGW
En los ajustes del compilador (Linker), debes añadir: -lpthread.

## Ejecución

```bash
./chat <puerto> <ipv4> <ipv6>
```

## Ejemplo

```bash
./chat 8080 172.16.18.1 2001:acad:cafe::1
```

## Argumentos

- `puerto`: puerto utilizado por el chat
- `ipv4`: dirección IPv4 del cliente
- `ipv6`: dirección IPv6 del cliente

## Requisitos

- GCC
- Linux
- cygwin1.dll o algun programa que cuente con MinGW