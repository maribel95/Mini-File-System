/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
#include "ficheros.h"
#define tambuffer 1500 // Constante simbólica del tamaño del buffer

int main(int argc, char **argv){
    // Variables
    unsigned char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    int ninodo = atoi(argv[2]); 
    int offset = 0;
    if(argc != 3){
        fprintf(stderr,"Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>.\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Leemos del inodo
    int bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
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
        bytesleidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    }
    // Leemos los datos del inodo
    struct STAT stat;
    mi_stat_f(ninodo,&stat); 
    // Imprimimos por pantalla el total de bytes leídos 
    fprintf(stderr,"\nTotal_leidos= %d",bytestotal);
    // Imprimimos también el tamaó en bytes lógico del inodo leído
    fprintf(stderr,"\nTamEnBytesLog= %d \n",stat.tamEnBytesLog);

    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }

    return 0;
}

