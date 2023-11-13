/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Cabeceras
#include "directorios.h"
#include "semaforo_mutex_posix.h"
#include <sys/wait.h>
#include <signal.h>
// Constantes
#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000
// Estructuras
struct REGISTRO { //sizeof(struct REGISTRO): 24
   time_t fecha; //Precisión segundos
   pid_t pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el número de escritura (de 1 a 50)
   int nRegistro; //Entero con el número del registro dentro del fichero
};
// Funciones
void reaper(); // Señal del enterrador
