/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
#include "ficheros.h"

int main(int argc, char **argv){
    // Comprobamos si el número de argumentos introducido por parámetros es correcto
    if(argc != 4){
        fprintf(stderr,"Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // El número de bytes nos viene como parámetro por consola
    int nbytes = atoi( argv[3] ); 
    // El número de inodo también nos llega como parámetro por consola
    int ninodo = atoi( argv[2] ) ;
    if(nbytes == 0){
        // Si el inodo está vacío, simplemente liberamos el inodo
        liberar_inodo(ninodo);
    }else{
        // Sino, truncamos la cantidad de bytes pasada por parámetro
        mi_truncar_f(ninodo,nbytes);
    }
    // Leemos el estado del inodo para comprobar si todo es correcto
    struct STAT stat;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    mi_stat_f(ninodo,&stat);
    // Comprobamos todos los datos del inodo
    printf("DATOS INODO\n");
    // Imprimimos datos varios
    printf("tipo=%c\n",stat.tipo);
    printf("permisos=%d\n",stat.permisos);
    ts = localtime(&stat.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	fprintf(stdout,"ATIME: %s \nMTIME: %s \nCTIME: %s\n",atime,mtime,ctime);
    printf("nlinks=%d\n",stat.nlinks);
    // Comprobamos el estado de tamEnBytesLog
    printf("tamEnBytesLog=%d\n",stat.tamEnBytesLog);
    // Comprobamos el estado de numBloquesOcupados
    printf("numBloquesOcupados=%d\n",stat.numBloquesOcupados);
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
}