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
    if(argc != 5){
        // Imprimimos estos mensajes de error
        fprintf(stderr,"Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }
    // El offset introducido estará en el argumento 4
    int offset = atoi(argv[4]);
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Variables
    int longitud = strlen(argv[3]);
    // Imprimimos la longitud del texto
    fprintf(stdout,"Longitud texto: %d\n",longitud);
    char *rutaFichero = argv[2];
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    int bytesEscritos = mi_write(rutaFichero, argv[3], offset, longitud * sizeof(char));
    if(bytesEscritos < 0){
        //fprintf(stderr,"Error en la escritura de la capa de directorios.\n");
        printf("Bytes escritos: 0\n");
        return -1;
    }
    fprintf(stdout,"Bytes escritos: %d\n",bytesEscritos);
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
    return 0;
}