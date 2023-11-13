/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Cabecera correspondiente
#include "simulacion.h"
static int acabados = 0;

int main(int argc, char **argv){
    signal(SIGCHLD, reaper); // Asociamos la señal de cuando muere el hijito al enterrador
    // Validamos que el número de argumentos sea correcto
    if(argc != 2){
        // Imprimimos sintaxis
        fprintf(stderr,"Sintaxis: ./simulacion <disco>\n");
        return -1;
    }
    char *nombreDispositivo = argv[1];
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(nombreDispositivo) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Creamos directorio de la simulación
    char *dirSimulacion = malloc(20);
    // Copiamos la primera parte del directorio en el espacio reservado para el directorio
    strcat(dirSimulacion,"/simul_");
    // Incorporamos información acerca del tiempo en el nombre del directorio
    struct tm *tm; 
    time_t tiempoActual;
    tiempoActual = time(NULL);
    char tmp[80];
    tm = localtime(&tiempoActual);
    sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(dirSimulacion,tmp);
    strcat(dirSimulacion,"/"); // Concatenamos barrita del final porque es un directorio
    if(mi_creat(dirSimulacion, 6) < 0){ // Creamos el directorio principal de la simulación
        fprintf(stderr,"Error al intentar crear un directorio.\n");
        return -1;
    }
    // Ahora creamos los procesos
    for(int procesos = 1; procesos <= NUMPROCESOS; procesos++){
        pid_t pid = fork(); // Creamos un proceso nuevo
        if(pid == 0){ // Proceso hijo
            // Montamos de nuevo el dispositivo para que el hijo tenga un descriptor
            // distinto al hacer un open y así evitar problemas de concurrencia
            if(bmount(nombreDispositivo) == -1){
                fprintf(stderr,"Error en el montaje del dispositivo\n");
                return -1;
            }   
            // Creamos el directorio
            char *nombreDirectorio = malloc(TAMNOMBRE);
            char *nombrePID = malloc(TAMNOMBRE);
            strcpy(nombreDirectorio,"proceso_"); // Parte inicial del nombre
            sprintf(nombrePID,"%d",getpid()); // Convertimos el pid a un String
            strcat(nombreDirectorio,nombrePID); // Concatenamos
            strcat(nombreDirectorio,"/"); // Le añadimos una barra porque es un directorio
            // Ruta en la que hay que crear todos los directorios y ficheros
            char *ruta = malloc(TAMNOMBRE*PROFUNDIDAD); // Tamaño máximo que puede tener una ruta
            strcpy(ruta,dirSimulacion); // Concatenamos el nombre del directorio principal de la simulacion
            strcat(ruta,nombreDirectorio); // Concatenamos el nombre del directorio del proceso en particular
            if(mi_creat(ruta, 6) < 0){ // Creamos el directorio
                fprintf(stderr,"Error al intentar crear un directorio.\n");
                return -1;
            }
            strcat(ruta,"prueba.dat"); // Concatenamos el nombre del fichero a la ruta
            if(mi_creat(ruta, 6) < 0){ // Creamos el fichero
                fprintf(stderr,"Error al intentar crear un directorio.\n");
                return -1;
            }
            // Ahora inicializamos la semilla de números aleatorios
            srand(time(NULL) + getpid());
            // Definimos la variable del registro
            struct REGISTRO registro;
            // Realizamos todas las escrituras en el fichero
            for(int nEscritura = 1; nEscritura <= NUMESCRITURAS; nEscritura++){
                // Inicializamos el registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nEscritura;
                registro.nRegistro = rand() % REGMAX;
                // Escribimos el registro
                if(mi_write(ruta, &registro, registro.nRegistro*sizeof(struct REGISTRO),sizeof(struct REGISTRO)) == -1){
                    fprintf(stderr,"Error al escribir el registro en el fichero.\n");
                    return -1;
                }
                //printf("[simulación.c → Escritura %d en %s]\n",nEscritura, ruta);
                // Esperamos 0,05 segundos para la proxima escritura
                usleep(50000); // 50.000 * 10^-6 = 0,05 segundos                
            }
            printf("[Proceso %d: Completadas %d escrituras en %s]\n",procesos,NUMESCRITURAS,ruta);
            // Desmontamos el dispositivo
            if(bumount() == -1){ // Desmontamos el dispositivo del hijito
                fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
                return -1;
            }
            // Liberamos el espacio de memoria reservado anteriormente para la creación de la ruta del directorio
            free(nombreDirectorio);
            free(nombrePID);
            free(ruta);
            exit(0); // Emitimos señal SIGCHLD
        }
        // Esperamos 0,2 segundos para lanzar el siguiente proceso
        usleep(200000); // 200.000 * 10^-6 = 0,2 segundos        
    }
    // Mientras no hayan acabado todos los procesos, no avanzaremos en código
    while(acabados < NUMPROCESOS){
        pause();
    }
    // Desmontamos el dispositivo
    if(bumount() == -1){ // Desmontamos el dispositivo del padre
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    } 
    // liberamos el directorio de la simulacion
    free(dirSimulacion);
    exit(0); // Fin      
}
// Función enterrador de hijos
void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
    }
}
