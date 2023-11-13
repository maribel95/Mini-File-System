/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la última cabecera en ser creada
#include "directorios.h"

int main(int argc, char **argv){
    // Obtenemos el nombre del fichero
    char *nombreFichero = argv[1];
    // Obtenemos el número de bloques del sistema
    nbloques = atoi(argv[2]);
    int ninodos = nbloques/4;
    // Array de 0,s para inicializar el sistema de tamaño de 1 bloque
    unsigned char arrayCeros[BLOCKSIZE];
    memset(arrayCeros,0,BLOCKSIZE);
    // Montamos el dispositivo virtual con el nombre del fichero
    bmount(nombreFichero);
    // Ahora inicializamos los bloques
    for( int i = 0; i < nbloques; i++){
        bwrite(i,arrayCeros);
    }
    initSB(nbloques,ninodos);
    initMB();
    initAI();
    reservar_inodo('d',7);
    
    // Cerramos el fichero
    bumount();
    
    return 0;
}

