/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera
#include "directorios.h"

int main(int argc, char **argv){
    // Comprobamos que el número de comandos coincida con la sintaxis
    if(argc != 4){ 
        // Imprimimos cómo debería ser
        fprintf(stderr,"Sintaxis: ./mi_touch <disco> <permisos> </ruta>\n");
        return -1;
    }
    // Los permisos están en el tercer parámetro que nos llega por consola
    int permisos = atoi(argv[2]);
    // Comprobamos si se han introducido unos permisos válidos
    if(permisos > 7 || permisos < 0){
        fprintf(stderr,"Error, se han introducido unos permisos incorrectos.\n");
        return -1;
    }

    // La ruta vendrá como cuarto parámetro que nos llega por consola
    char *ruta = argv[3];
    // Calculamos la longitud de esta ruta
    int longitud_ruta = strlen(ruta);
    // Miramos si la penúltima posición del String( la última es el símbolo de fin de carácter), coincide con una /
    if(ruta[longitud_ruta - 1] != '/'){ // Si no coincide, entonces es un fichero
        // Montamos el dispositivo con el nombre pasado por parámetro
        if(bmount(argv[1]) == -1){
            fprintf(stderr,"Error en el montaje del dispositivo\n");
            return -1;
        }
        // Creamos el fichero
        mi_creat(ruta, permisos);
        // Desmontamos el dispositivo
        if(bumount() == -1){
            fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
            return -1;
        }
        return 0;
    }else{ // No era un fichero y por lo tanto no se ha podido crear correctamente.
        fprintf(stderr,"No se ha podido crear el fichero.\n");
        return -1;
    }
}