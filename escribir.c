/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
#include "ficheros.h"

int main(int argc, char **argv){
    // Declaramos un array de offsets a probar
    int offset[] = {9000,209000,30725000,409605000, 480000000};
    // validamos que el número de argumentos sea correcto
    if(argc != 4){
        // Imprimimos estos mensajes de error
        fprintf(stderr,"Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr,"Offsets: %d, %d, %d, %d, %d\n", offset[0], offset[1], offset[2], offset[3], offset[4]);
        fprintf(stderr,"Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Variables
    int longitud = strlen(argv[2]);
    char *buffer = argv[2];
    int diferentes = atoi(argv[3]);
    // Reservamos un inodo
    int ninodo = reservar_inodo('f',6);
    printf("Número de inodo reservado: %d\n",ninodo);

    if(ninodo == -1){ // Recogemos posible error
        fprintf(stderr,"Error al reservar inodo.\n");
        return -1;
    }
    // Variables para saber la información de los inodos
    struct STAT stat;
    int elementosOffset = sizeof(offset) / sizeof(offset[0]);
    // Imprimimos la longitud del texto
    fprintf(stdout,"Longitud texto: %d\n",longitud);
    // Imprimimos todos los Offsets
    fprintf(stdout,"Offsets: ");
    for( int i = 0 ; i < elementosOffset ; i++ ){
        fprintf(stdout,"%d ",offset[i]);
    }
    // Hacemos la escritura para cada uno de los diferentes Offsets.
    for(int i = 0; i < elementosOffset ;i++){
        // Imprimimos el inodo reservado y el offset que toque
        fprintf(stdout,"\n\nNº inodo reservado : %d\n",ninodo);
        fprintf(stdout,"Offset : %d\n",offset[i]);
        // Imprimimos los bytes escritos en el inodo
        fprintf(stdout,"Bytes escritos: %d \n",mi_write_f(ninodo,buffer,offset[i],longitud * sizeof(char)));
        mi_stat_f(ninodo,&stat);
        fprintf(stdout,"stat.tamEnBytesLog: %d \n",stat.tamEnBytesLog);
        fprintf(stdout,"stat.numBloquesOcupados: %d \n",stat.numBloquesOcupados);
        // Comprobamos si tenemos que reservar mas inodos o no
        // También tenemos la condición para no reservar un inodo de más
        if(diferentes != 0 && i < elementosOffset - 1){
            ninodo = reservar_inodo('f',6);
            if(ninodo == -1){ // Comprobamos que no haya ningún error
                fprintf(stderr,"Error al reservar inodo.\n");
                return -1;
            }
        }
    }
    // Finalmente limpiamos el buffer y lo ponemos todo a 0
    memset(buffer,0,longitud * sizeof(char));
    // Imprimimos los bytes leidos
    fprintf(stdout,"bytes leídos: %d \n\n",mi_read_f(ninodo,buffer,offset[1],longitud * sizeof(char)));
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
    return 0;
}

