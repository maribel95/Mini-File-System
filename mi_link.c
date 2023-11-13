/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera correspondiente
#include "directorios.h"

int main(int argc, char **argv){

    // validamos que el número de argumentos sea correcto
    if(argc != 4){
        // Imprimimos estos mensajes de error
        fprintf(stderr,"Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
        return -1;
    }

    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    char *camino1 = argv[2];
    char *camino2 = argv[3];
    // Obtenemos la ruta y comprobamos que no se refiera a un directorio
    if (camino1[strlen(camino1)-1] == '/') {
        fprintf(stderr, "Error: la ruta de camino1 se corresponde a un directorio\n");
        return -1;
    }
    // Obtenemos la ruta y comprobamos que no se refiera a un directorio
    if (camino2[strlen(camino2)-1] == '/') {
        fprintf(stderr, "Error: la ruta de camino2 se corresponde a un directorio\n");
        return -1;
    }
    // Llamamos a mi_link de la capa de directior
    if (mi_link(camino1,camino2) < 0) { return -1;}

    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
    return 0;
}