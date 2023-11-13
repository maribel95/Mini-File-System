/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Cabeceras
#include "bloques.h"
#include "semaforo_mutex_posix.h"
// Variables globales
static int descriptor = 0;
static unsigned int inside_sc = 0;
static sem_t *mutex;
// Función para montar el dispositivo virtual
int bmount(const char *camino){
    // Los procesos hijos pueden heredar el mismo descriptor del padre, así que hacemos esto
    // para evitar posibles problemas de concurrencia
    if (descriptor > 0) {
       close(descriptor);
    }
    umask(000); // Máscara para controlar el modo de creación de ficheros
    descriptor = open(camino,O_RDWR|O_CREAT,0666);
    // Capturamos posible fallo al abrir el fichero
    if(descriptor == -1){
        // Imprimimos fallo
        fprintf(stderr,"Fallo al abrir el fichero.\n");
        return -1; // Devolvemos fallo
    }
    // Inicializamos el mutex en el montaje del dispositivo
    if (!mutex) { // Si no estaba inicialicado de antes, lo hacemos ahora
        mutex = initSem(); // Función para inicializar el mutex
        if (mutex == SEM_FAILED) { // En caso de fallo, devolvemos error
           return -1;
        }
    }
    return descriptor;
}

// Función para desmontar el dispositivo virtual
int bumount(){
    // Cerramos el descriptor
    descriptor = close(descriptor);
    int cerrarFichero = close(descriptor);
    // Capturamos fallo al cerrar el fichero
    if (cerrarFichero == -1){
        // Imprimimos el error
        fprintf(stderr,"Fallo al cerrar el fichero.\n");
        return -1; // Devolvemos fallo
    }
    // Borramos el semáforo
    deleteSem(); 
    // Si todo ha ido bien
    return 0;
}
// Función para escribir en el fichero
int bwrite(unsigned int nbloque, const void *buf){
    // Lugar donde queremos escribir el bloque 
    int desplazamiento = nbloque * BLOCKSIZE; 
    // Colocamos el puntero del descriptor donde corresponda
    if( lseek(descriptor,desplazamiento,SEEK_SET) == -1 ){
        // Imprimimos error
        fprintf(stderr,"Error al posicionar el puntero del descriptor.\n");
        return -1;
    }
    // Y ahora escribimos un bloque
    size_t bytesEscritos = write(descriptor,buf,BLOCKSIZE);
    if( bytesEscritos == -1){
        // Imprimimos error
        fprintf(stderr,"Error al intentar escribir en el fichero.\n");
        return -1;
    }
    // Devolvemos los bytes que hemos escrito en el fichero
    return bytesEscritos;
}
// Función para leer el fichero
int bread(unsigned int nbloque, void *buf){
    // Lugar donde queremos leer el bloque 
    int desplazamiento = nbloque * BLOCKSIZE; 
    // Colocamos el puntero del descriptor donde corresponda
    if( lseek(descriptor,desplazamiento,SEEK_SET) == -1 ){
        // Imprimimos error
        fprintf(stderr,"Error al posicionar el puntero del descriptor.\n");
        return -1;
    }
    // Y ahora escribimos un bloque
    size_t bytesLeidos = read(descriptor,buf,BLOCKSIZE);
    if( bytesLeidos == -1){
        // Imprimimos error
        fprintf(stderr,"Error al intentar leer el fichero.\n");
        return -1;
    }
    // Devolvemos los bytes que hemos escrito en el fichero
    return bytesLeidos;

}
// Para evitar código reentrante
void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}
// Para evitar código reentrante
void mi_signalSem() {
   inside_sc--;
    if (!inside_sc) {
       signalSem(mutex);
    }
}







