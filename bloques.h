/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// bloques.h
 
#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()
 
#define BLOCKSIZE 1024 // bytes
// Nivel 1
int bmount(const char *camino); // Función para montar el dispositivo
int bumount(); // Función para desmontarlo
int bwrite(unsigned int nbloque, const void *buf); // Función para escribir en el dispositivo
int bread(unsigned int nbloque, void *buf); // Función para leer del dispositivo
// Nivel 11
void mi_waitSem(); // Espera semaforo
void mi_signalSem(); // Señal semaforo
