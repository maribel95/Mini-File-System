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
        fprintf(stderr,"Sintaxis: ./mi_mv <disco> </origen/nombre> </destino/>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Varuables con los nombres de las rutas
    char *rutaOrigen = argv[2];
    char *destino = argv[3];
    // Comprobamos que el destino sea un directorio
    if(destino[strlen(destino) - 1] != '/'){
        fprintf(stderr,"Error: El destino tiene que ser un directorio.\n");
        return -1;
    }
    // Variables de los inodos de la ruta destino para buscar entrada
    unsigned int p_inodo_dir_destino = 0;
    unsigned int p_inodo_destino = 0;
    unsigned int p_entrada_destino = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(destino,&p_inodo_dir_destino,&p_inodo_destino,&p_entrada_destino,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    // Variables de los inodos de la ruta origen para buscar entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    errores = buscar_entrada(rutaOrigen,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    struct inodo inodo;
    if(leer_inodo(p_inodo_destino,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    }
    // Guardamos la entrada del inodo de donde habrá que borrarla
    struct entrada entrada;

    // Leemos la entrada correspondiente que hay que mover de sitio
    if(mi_read_f(p_inodo_dir,&entrada,p_entrada * sizeof(struct entrada),sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // Leemos el inodo asociado a la ruta de origen
    struct inodo inodoOrigen;
    if(leer_inodo(p_inodo_dir,&inodoOrigen) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    }  
    // Cálculamos cuántas entradas tiene el inodo destino donde queremos añadir la nueva
    int entradas_inodoOrigen = inodoOrigen.tamEnBytesLog / sizeof(struct entrada);
    struct entrada ultimaEntrada;
    // Leemos la última entrada
    if(mi_read_f(p_inodo_dir,&ultimaEntrada,entradas_inodoOrigen * sizeof(struct entrada),sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // Escribimos la última entrada en el lugar donde hay que borrar la que queremos trasladar
    if(mi_write_f(p_inodo_dir,&ultimaEntrada,p_entrada * sizeof(struct entrada),sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // Truncamos el inodo del origen, quitándole la entrada correspondiente
    mi_truncar_f(p_inodo_dir, inodoOrigen.tamEnBytesLog-sizeof(struct entrada));
    // Cálculamos cuántas entradas tiene el inodo destino donde queremos añadir la nueva
    int entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
    // Escribimos en el inodo destino la nueva entrada 
    if(mi_write_f(p_inodo_destino,&entrada,(entradas_inodo) * sizeof(struct entrada),sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }
    return 0; // Todo ha salido bien     
}