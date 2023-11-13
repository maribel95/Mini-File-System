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
        fprintf(stderr,"Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // Varuables con los nombres de las rutas
    char *rutaOriginal = argv[2];
    char *nombreNuevo = argv[3];
    // Variables de los inodos para buscar entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(rutaOriginal,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    // Leemos el inodo asociado al directorio padre
    struct inodo inodo;
    if(leer_inodo(p_inodo_dir,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    }
    // Cálculamos cuántas entradas tiene el inodo padre
    int entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
    // Buffer auxiliar que almacenará todas las entradas del inodo padre
    struct entrada bufferEntradas[entradas_inodo];
    // Leemos todas las entradas del inodo que buscamos
    if(mi_read_f(p_inodo_dir,bufferEntradas,0,sizeof(struct entrada)*entradas_inodo) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    // Miramos a ver si hay alguna entrada en ese directorio que coincida con la que ya tenemos
    for(int i = 0; i < entradas_inodo; i++){
        if(strcmp(bufferEntradas[i].nombre,nombreNuevo) == 0){
            fprintf(stderr,"No se puede renombrar, el nombre ya existe en este directorio.\n");
            return -1;
        }
    }
    // Si hemos llegado hasta aquí, es porque ningún nombre coincidía y por lo tanto podemos hacer el cambio
    // Cambiamos el nombre
    strcpy(bufferEntradas[p_entrada].nombre,nombreNuevo);
    // Y lo escribimos, esta vez actualizado
    if(mi_write_f(p_inodo_dir,&bufferEntradas[p_entrada],p_entrada * sizeof(struct entrada),sizeof(struct entrada)) == -1){
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