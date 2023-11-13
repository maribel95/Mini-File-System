/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera correspondiente
#include "directorios.h"
int borrarDirectoriosRecursivo(char *ruta,unsigned int p_inodo);

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
    // Comprobamos que no sea un fichero
    if(ruta[strlen(ruta) -1] != '/') {
        fprintf(stderr, "Error: no se ha introducido un fichero.\n");
        return -1;
    }

    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    // Variables de los inodos para buscar entrada
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(ruta,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    // Llamamos a la función recursiva que nos borrará todo el directorio
    borrarDirectoriosRecursivo(ruta,p_inodo);

    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo.\n");
        return -1;
    }
    return 0;
}

int borrarDirectoriosRecursivo(char *ruta,unsigned int p_inodo){
    // Leemos el inodo asociado a la entrada del camino
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al leer inodo.\n");
        return -1;       
    }
    char *rutaAux;
    // Calculamos cuantas entradas tiene el inodo del directorio
    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    // El directorio está vacío o es un fichero
    if((inodo.tipo == 'd' && numEntradas == 0 )|| inodo.tipo == 'f'){
        // Llamamos a mi un_link de la capa de directorios
        if(mi_unlink(ruta) == -1){
            fprintf(stderr, "Error: no se ha podido borrar la entrada.\n");
            return -1;
        }
        return 0;
    }
    // Buffer auxiliar que almacenará todas las entradas del inodo
    struct entrada buffEntradas[numEntradas];
    // Leemos todas las entradas
    if(mi_read_f(p_inodo,buffEntradas,0,sizeof(struct entrada) * numEntradas) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // El directorio no está vacío y hay que ir mirando sus entradas una por una
    for(int i = 0; i < numEntradas; i++){

        struct inodo inodoEntrada;
        if(leer_inodo(buffEntradas[i].ninodo,&inodoEntrada) == -1){
            fprintf(stderr,"Error al leer inodo.\n");
            return -1;       
        }
        // En caso de que sea un directorio, entonces también le concatenamos la barra del final  
        if(inodoEntrada.tipo == 'd'){
            rutaAux = malloc(strlen(ruta) + 1 + strlen(buffEntradas[i].nombre) + 1); // Variable cuya función es hacer de cadena auxiliar  
            strcpy(rutaAux,ruta); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
            // Creamos una nueva ruta donde buscar
            strcat(rutaAux,buffEntradas[i].nombre);  
            strcat(rutaAux,"/");
        }else{ // Si es un fichero
            rutaAux = malloc(strlen(ruta) + 1 + strlen(buffEntradas[i].nombre)); // Variable cuya función es hacer de cadena auxiliar  
            strcpy(rutaAux,ruta); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
            // Creamos una nueva ruta donde buscar
            strcat(rutaAux,buffEntradas[i].nombre);              
        }
        // Llamamos a la función recursiva pasandole la nueva ruta
        borrarDirectoriosRecursivo(rutaAux,buffEntradas[i].ninodo);
        free(rutaAux);
    }
    // Llamamos a mi un_link de la capa de directorios
    if(mi_unlink(ruta) == -1){
        fprintf(stderr, "Error: no se ha podido borrar la entrada.\n");
        return -1;
    }    
    return 0;
}

