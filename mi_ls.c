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
    if(argc != 3){ 
        // Imprimimos cómo debería ser
        fprintf(stderr,"Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return -1;
    }

    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }

    char buff[TAMBUFFER];
    char tipo;
    char *camino = argv[2];
    // Comprobamos que la ruta que nos han pasado es un directorio o en un fichero
    if(camino[strlen(camino)-1] == '/'){ // Es un directorio
        tipo = 'd';
    }else{ // Es un fichero
        tipo = 'f';
    }
    // Llamamos a la función que imprimira los datos del camino
    mi_dir(camino,buff,tipo);
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }
    return 0;
}


