/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera
#include "directorios.h"
int crearRecursivo(char *ruta,char *destino, unsigned int p_inodo);

int main(int argc, char **argv){
    // Comprobamos que el número de comandos coincida con la sintaxis
    if(argc != 4){ 
        // Imprimimos cómo debería ser
        fprintf(stderr,"Sintaxis: ./mi_cp <disco> <origen> <destino>\n");
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

    // Variables de los inodos de la ruta origen para buscar entrada
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(rutaOrigen,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    struct entrada entrada;
    // Leemos todas las entradas
    if(mi_read_f(p_inodo_dir,&entrada,p_entrada* sizeof(struct entrada),sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    char *rutaDestino;
    rutaDestino= malloc(strlen(destino) + 1 + strlen(entrada.nombre) + 1); // Variable cuya función es hacer de cadena auxiliar  
    strcpy(rutaDestino,destino); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
    // Creamos una nueva ruta donde buscar
    strcat(rutaDestino,entrada.nombre);  
    if(rutaOrigen[strlen(rutaOrigen) - 1] == '/'){
        strcat(rutaDestino,"/");
    }
    // Llamamos a esta función recursiva para copiar los elementos de la ruta original a la destino
    crearRecursivo(rutaOrigen,rutaDestino,p_inodo);

    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    }
    return 0; // Todo ha salido bien     
}


// Función auxiliar recursiva que irá creando los ficheros y directorios que hay que duplicar
int crearRecursivo(char *ruta,char *destino, unsigned int p_inodo){
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    }
    // Creamos una nueva entrada en el directorio que toca
    int p_inodoDestino = mi_creat(destino, inodo.permisos);
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    }
    // Comprobamos que no haya habido ningún error
    if(p_inodoDestino == -1){
        fprintf(stderr,"Error al intentar crear un fichero o directorio nuevo.\n");
        return -1;
    }
    // Obtenemos el p_inodo de la ruta destino
    struct inodo inodoDestino;
    if(leer_inodo(p_inodoDestino,&inodoDestino) == -1){
        fprintf(stderr,"Error al intentar leer inodo de la ruta original.\n");
        return -1;
    } 
    // Tenemos que copiar un fichero y por lo tanto hay que escribir texto dentro
    if(inodoDestino.tipo == 'f'){ // La ruta acaba en fichero     
        char bufferLectura[inodo.tamEnBytesLog];
        // Limpiamos de basura el buffer
        memset(bufferLectura,0, sizeof(bufferLectura));
        // Leemos la información de la ruta original al completo
        if(mi_read(ruta,bufferLectura,0,sizeof(bufferLectura)) == -1){
            fprintf(stderr,"Error al leer el fichero.\n");
            return -1;
        }
        // Ahora queremos calcular cuantos offsets hay, cuales son y el tamaño del texto que hay en ese offset
        int numOffsets = 0;
        int primeraPosicionEscritura = 0;
        int bufferOffsets[100];
        int bufferLongitud[100];
        memset(bufferLongitud,0,sizeof(bufferLongitud));
        // Realizamos un recorrido de todo el inodo para encontrar donde estan esos offsets
        for(int i = 0, j = 0, k = 0; i < inodo.tamEnBytesLog - 1; i++){
            if(strcmp(&bufferLectura[i],"\0") != 0 ){ // Comprobamos que lo que leemos es texto
                if(primeraPosicionEscritura == 0){ // Coindición para averiguar donde se halla el primer offset
                    k = numOffsets;
                    numOffsets++;
                    bufferOffsets[j] = i;
                    j++;
                } 
                primeraPosicionEscritura = 1;
                bufferLongitud[k]++;                  
            }else{ // Si lo que leemos es basurilla
                primeraPosicionEscritura = 0;
            }
        }
        // Buffer para realizar las escrituras y lecturas
        char bufferDestino[BLOCKSIZE];
        // Lo hacemos tantas veces como offsets tuviera el inodo original
        for(int i = 0; i < numOffsets; i++){
            // Limpiamos de basura el buffer
            memset(bufferDestino,0, sizeof(bufferDestino));
            // Leemos la información de la ruta original
            if(mi_read(ruta,bufferDestino,bufferOffsets[i],bufferLongitud[i] + 1) == -1){
                fprintf(stderr,"Error al leer el fichero.\n");
                return -1;
            }
            // Escribimos el contenido en el nuevo destino
            if(mi_write(destino,bufferDestino,bufferOffsets[i],bufferLongitud[i] + 1) == -1){
                fprintf(stderr,"Error al leer el fichero.\n");
                return -1;
            }  
        }
    }else{ // La ruta es un directorio
        int entradasDirectorioOriginal = inodo.tamEnBytesLog / sizeof(struct entrada);
        // Buffer auxiliar que almacenará todas las entradas del inodo
        struct entrada buffEntradas[entradasDirectorioOriginal];
        // Leemos todas las entradas
        if(mi_read_f(p_inodo,buffEntradas,0,sizeof(struct entrada) * entradasDirectorioOriginal) == -1){
            fprintf(stderr,"Error al leer el fichero.\n");
            return -1;
        }
        char *rutaAux;
        char *rutaAuxDestino;
        for(int i = 0; i < entradasDirectorioOriginal; i++){
            struct inodo inodoEntrada;
            if(leer_inodo(buffEntradas[i].ninodo,&inodoEntrada) == -1){
                fprintf(stderr,"Error al leer inodo.\n");
                return -1;       
            }
            // En caso de que sea un directorio, entonces también le concatenamos la barra del final  
            if(inodoEntrada.tipo == 'd'){
                // Creamos la ruta Origen
                rutaAux = malloc(strlen(ruta) + 1 + strlen(buffEntradas[i].nombre) + 1); // Variable cuya función es hacer de cadena auxiliar  
                strcpy(rutaAux,ruta); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
                // Creamos una nueva ruta donde buscar
                strcat(rutaAux,buffEntradas[i].nombre);  
                strcat(rutaAux,"/");
                // Creamos la ruta de destino
                rutaAuxDestino = malloc(strlen(destino) + 1 + strlen(buffEntradas[i].nombre) + 1); // Variable cuya función es hacer de cadena auxiliar  
                strcpy(rutaAuxDestino,destino); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
                // Creamos una nueva ruta donde buscar
                strcat(rutaAuxDestino,buffEntradas[i].nombre);  
                strcat(rutaAuxDestino,"/");
            }else{ // Si es un fichero
                // Creamos la ruta Origen
                rutaAux = malloc(strlen(ruta) + 1 + strlen(buffEntradas[i].nombre)); // Variable cuya función es hacer de cadena auxiliar  
                strcpy(rutaAux,ruta); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
                // Creamos una nueva ruta donde buscar
                strcat(rutaAux,buffEntradas[i].nombre); 
                // Creamos la ruta de destino  
                rutaAuxDestino = malloc(strlen(destino) + 1 + strlen(buffEntradas[i].nombre)); // Variable cuya función es hacer de cadena auxiliar  
                strcpy(rutaAuxDestino,destino); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
                // Creamos una nueva ruta donde buscar
                strcat(rutaAuxDestino,buffEntradas[i].nombre);       
            }
            // Llamamos a la función recursiva pasandole la nueva ruta
            crearRecursivo(rutaAux,rutaAuxDestino, buffEntradas[i].ninodo);
            free(rutaAux);
            free(rutaAuxDestino);
        }
    }
    return 0; // Todo ha salido bien
}