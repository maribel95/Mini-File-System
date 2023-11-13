/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
#include "directorios.h"
#define tambuffer 1700 // Constante simbólica del tamaño del buffer

int main(int argc, char **argv){
    // Variables
    unsigned char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    int offset = 0;
    if(argc != 3){
        fprintf(stderr,"Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }

    // Obtenemos la ruta y comprobamos que no se refiera a un directorio
    if (argv[2][strlen(argv[2])-1]=='/') {
        fprintf(stderr, "Error: la ruta se corresponde a un directorio.\n");
        return -1;
    }
    // Leemos del inodo mediante la función mi_read de la capa de ficheros
    int bytesleidos = mi_read(argv[2], buffer_texto, offset, tambuffer);
    // Variable que almacenará la cantidad total de bytes que vayamos leyendo
    int bytestotal = 0;
    // Vamos leyendo todo el texto
    while( bytesleidos > 0 ){
        // El offset va sumando caracteres
        offset += tambuffer;
        // Escribimos por la salida estandar lo que leemos
        if( write(1, buffer_texto, bytesleidos) == -1 ){
            fprintf(stderr,"Error. No se ha podido realizar la escritura correctamente.\n");
            return -1;
        }
        // Aumentamos la cantidad total de bytes leídos
        bytestotal +=  bytesleidos;
        // Volvemos a limpiar el buffer para filtrar basura
        memset(buffer_texto, 0, tambuffer);
        // Y volvemos a leer del inodo
        bytesleidos = mi_read(argv[2], buffer_texto, offset, tambuffer);
    }
    // Imprimimos por pantalla el total de bytes leídos 
    fprintf(stderr,"\nTotal_leidos %d\n",bytestotal);

    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }

    return 0;
}