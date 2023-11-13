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
    // Comprobamos que el número de comandos coincida con la sintaxis
    if(argc != 4){ 
        // Imprimimos cómo debería ser
        fprintf(stderr,"Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n");
        return -1;
    }
    int permisos = atoi(argv[2]);
    // Comprobamos que se hayan introducido permisos dentro del rango correcto
    if(permisos > 7 || permisos < 0){
        fprintf(stderr,"Error, se han introducido permisos incorrectos.\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Cambiamos permisos
    if(mi_chmod(argv[3],permisos) < 0){
        fprintf(stderr,"Error al intentar cambiar los permisos.\n");
        return -1;
    }

    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }

    return 0;
}