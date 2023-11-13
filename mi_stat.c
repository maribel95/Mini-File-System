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
        fprintf(stderr,"Sintaxis: ./mi_stat <disco> </ruta>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    struct STAT p_stat;

    // Llamamos a mi_stat e imprimimos la información
    if(mi_stat(argv[2],&p_stat) < 0){
        fprintf(stderr,"Error al intentar cambiar los permisos.\n");
        return -1;
    }

    char atime[80];
	char mtime[80];
	char ctime[80];
    struct tm *ts;
    printf("tipo: %c\n",p_stat.tipo);
    printf("permisos: %d\n",p_stat.permisos);
    ts = localtime(&p_stat.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&p_stat.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&p_stat.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	fprintf(stdout,"ATIME: %s \nCTIME: %s \nMTIME: %s\n",atime,ctime,mtime);
    printf("nlinks: %d\n",p_stat.nlinks);
    printf("tamEnBytesLog: %d\n",p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",p_stat.numBloquesOcupados);
    printf("\n");
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }
    return 0;
}


