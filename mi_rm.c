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
    if(argc != 3){
        // Imprimimos estos mensajes de error
        fprintf(stderr,"Sintaxis: ./mi_rm disco /ruta\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    char *ruta = argv[2];
    // Comprobamos que no nos hayan pasado el directorio raiz
    if(strcmp("/",ruta) == 0) {
        fprintf(stderr, "Error: no se puede borrar el directorio raíz.\n");
        return -1;
    }
    // Comprobamos que sea un directorio
    if(ruta[strlen(ruta) -1] == '/') {
        fprintf(stderr, "Error: no se ha introducido un fichero.\n");
        return -1;
    }
    // Llamamos a mi un_link de la capa de directorios
    if(mi_unlink(ruta) == -1){
        fprintf(stderr, "Error: no se ha podido borrar el fichero.\n");
        return -1;
    }
    char *caminoAux = malloc(sizeof(ruta));
    strncpy(caminoAux,ruta,5);
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
    return 0;
}