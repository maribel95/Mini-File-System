/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera correspondiente
#include "directorios.h"
// Definimos esta constante que refleja el numero de entradas que puede contener la mini memoria cache
#define entradasCache 10
// Creamos un array de las últimas entradas utilizadas para escritura para simular una pequeña memoria caché
static struct cache UltimaEntradaEscritura[entradasCache];
// Creamos un array de las últimas entradas utilizadas para lectura para simular una pequeña memoria caché
static struct cache UltimaEntradaLectura[entradasCache];
// Declaramos una variables globales que contarán los elementos del buffer de lectura y escritura
static int elementosBufferEscritura = 0;
static int elementosBufferLectura = 0;

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){ 
    // Recogemos posible error en caso de un directorio erroneo
    if(camino == NULL || camino[0] != '/'){return -1;}
    const char delimitador[2] = "/"; // El delimitador de un directorio es una barra
    int longitudCamino = strlen(camino);
    char *cadenaAux = malloc(longitudCamino + 1); // Variable cuya función es hacer de cadena auxiliar
    strcpy(cadenaAux,camino); // Copiamos camino a una cadena auxiliar para no tocar el contenido de dentro
    char *token = strtok(cadenaAux,delimitador); // Inicial contendrá el primer nombre de directorio que  venga en la ruta
    strcpy(inicial,token); // Copiamos el contenido del token aquí
    int longitudInicial = strlen(inicial) + 1; // Ahora calculamos la longitud del string contenido en inicial
    // y sumamos + 1 porque es como si tuvieramos en cuenta la barra
    // Hacemos un recorrido y vamos copiando un número de carácteres determinado. Nos podría haber sido útil la función strncpy,
    // pero como también queríamos empezar a copiar desde un offset del String( y eso la función no lo proporciona),
    // hemos decidido hacerlo así
    for(int i = longitudInicial,j = 0; i <= longitudCamino; i++,j++){ 
        final[j] = camino[i]; // Copiamos carácter a carácter
    }
    free(cadenaAux); // Liberamos la cadena auxiliar
    if(camino[longitudInicial] == '/'){ // Si encontramos la barra al final del string es un directorio
        *tipo = 'd';
        return 1; // Devolvemos 1 si es directorio
    }else{ // Sino, es un fichero
        *tipo = 'f';
        final = ""; // No puede ser el carácter '\0'
        return 0; // Devolvemos 0 si es fichero
    }
}

void mostrar_error_buscar_entrada(int error) {
    // Hacemos un switch por casos de los posibles errores
   switch (error) { 
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    // Variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;
    
    if(bread(posSB, &SB) == -1){
        fprintf(stderr,"Error al leer el super bloque\n.");
        return -1;
    }
    //si es directorio raiz entonces devuelve 0
    if (strcmp("/",camino_parcial) == 0) {
        *p_inodo = SB.posInodoRaiz; // Nuestra raiz está asociada al inodo 0
        *p_entrada = 0;
        return 0;
    }
    // Utilizamos la función extraer camino
    if(extraer_camino(camino_parcial,inicial,final,&tipo) == -1){
        // Error correspondiente a camino incorrecto
        return ERROR_CAMINO_INCORRECTO;
    }
    // Imprimimos los datos
    //printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n",inicial,final,reservar);
    // Buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir,&inodo_dir);
    // Miramos que cumpla los permisos de lectura
    if((inodo_dir.permisos & 4) != 4){
        // Imprimimos fallo
        //fprintf(stderr, "[buscar_entrada()→ El inodo %d no tiene permisos de lectura].\n",*p_inodo_dir);
        // Error correspondiente a los permisos de lectura
        return ERROR_PERMISO_LECTURA;        
    }
    // Ahora calculamos cuántas entradas tiene ese inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    if(cant_entradas_inodo > 0){
        // Inicializar buffer lectura a 0s
        memset(&entrada,0,sizeof(struct entrada));
        // Leemos la entrada
        mi_read_f(*p_inodo_dir,&entrada,num_entrada_inodo*sizeof(struct entrada),sizeof(struct entrada));
        while(num_entrada_inodo < cant_entradas_inodo && strcmp(entrada.nombre,inicial) != 0){
            // Aumentamos el numero del inodo de entrada
            num_entrada_inodo++;
            // Volvemos a poner el buffer de lectura a todo 0,s
            memset(&entrada,0,sizeof(struct entrada));
            // Leemos siguiente entrada
            mi_read_f(*p_inodo_dir,&entrada,num_entrada_inodo*sizeof(struct entrada),sizeof(struct entrada));
        }
    }
    if(num_entrada_inodo == cant_entradas_inodo && inicial != entrada.nombre){
        switch(reservar){
            case 0: // Modo consulta, como no existe, retornamos error
                // Error correspondiente a la consulta
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;break;
            case 1: // Modo escritura, creamos la entrada en el directorio referenciado por *p_inodo_dir
                if(inodo_dir.tipo == 'f'){ // Si es fichero no permitir escritura
                    // Devolvemos error de que no se pueden crear entradas si es un fichero
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                // Si es directorio comprobar que tiene permiso de escritura
                if((inodo_dir.permisos & 2) != 2){
                    // Devolvemos error de permisos de escritura
                    return ERROR_PERMISO_ESCRITURA;
                }else{ // Si tiene permisos de escritura
                    // Copiamos el nombre de inicial en la entrada
                    strcpy(entrada.nombre,inicial);
                    if(tipo == 'd'){ // Si es un directorio
                        if(strcmp("/", final) == 0){
                            // Reservamos un inodo como directorio y asignarlo a la entrada
                            entrada.ninodo =  reservar_inodo('d',permisos);
                            //printf("[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n",entrada.ninodo,permisos,inicial);
                            //printf("[buscar_entrada()→ creada entrada: %s, %d]\n",inicial,entrada.ninodo);
                        }else{ // Error
                            // Devolvemos error de que no existe el directorio intermedio
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;                        
                        }
                    }else{ // Es un fichero
                        // Reservar un inodo como fichero y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo('f',permisos);
                        //printf("[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n", entrada.ninodo, permisos, inicial);
                        //printf("[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, entrada.ninodo);  
                    }
                    // Escribir la entrada
                    if(mi_write_f(*p_inodo_dir,&entrada,num_entrada_inodo * sizeof(struct entrada),sizeof(struct entrada)) == -1){
                        // Se había reservado un inodo para la entrada entonces hay que liberarlo
                        if(entrada.ninodo != -1){
                            // Liberamos el inodo que habíamos asociado a la entrada
                            if(liberar_inodo(entrada.ninodo) == -1){
                                fprintf(stderr,"Error al liberar inodo.\n");
                                return -1;
                            }
                        }
                        // Devolvemos error de salida
                        return EXIT_FAILURE;
                    }
                }
            break;
        }
    }
    // Si hemos llegado al final del camino, será porque en final hay comillas o una barra
    if(strcmp(final,"") == 0 || strcmp(final,"/") == 0){
        if(num_entrada_inodo < cant_entradas_inodo && reservar == 1){
            // Modo escritura y la entrada ya existe
            // Devolvemos error de entrada ya existente
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // Asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_inodo = entrada.ninodo;
        // Asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *p_entrada = num_entrada_inodo;
        // Si hemos llegado hasta aquí es que todo ha salido bien
        return EXIT_SUCCESS;
    }else{
        // Asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada (final,p_inodo_dir,p_inodo,p_entrada,reservar,permisos);
    }
    // Todo ha salido bien
    return EXIT_SUCCESS;

}

int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem(); // Hacemos un wait del semáforo
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        mi_signalSem(); // Signal
        return -1;
    }
    //Inicialización de las variables que utilizaremos en extraer camino
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Hacemos buscar entrada con los siguientes parámetros iniciales
    int errores = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,1,permisos);
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        mi_signalSem(); // Signal
        return errores;
    }
    mi_signalSem(); // Señal
    return p_inodo;
}

int mi_dir(const char *camino, char *buffer, char tipo){
    mi_waitSem(); // Wait del semáforo
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        mi_signalSem(); // Señal semáforo
        return -1;
    }
    //Inicialización de las variables que utilizaremos en extraer camino
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        mi_signalSem(); // Señal semáforo
        return errores;
    }
    struct inodo inodo;
    // Leemos el inodo asociado al número de inodo correspondiente a la última entrada de camino
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer inodo.\n");
        mi_signalSem(); // Señal semáforo
        return -1;
    }
    //Comprobamos los permisos de lectura y que sea un directorio
    if ((inodo.permisos & 4) != 4){ 
		printf("Error: violación de privilegios \n");
        mi_signalSem(); // Señal semáforo
		return -1;
	}
    // Comprobamos que tiene sentido el tipo del inodo con el que nos han pasado como parámetro
    if(inodo.tipo != tipo){
        fprintf(stderr,"Error: la sintaxis no concuerda con el tipo\n");
        mi_signalSem(); // Señal semáforo
        return -1;
    }
    // Miramos el caso de si es un directorio, donde tendremos que listar todas sus entradas
    if(tipo == 'd'){
        // Cálculamos cuántas entradas tiene el inodo
        int entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
        // Buffer auxiliar que almacenará todas las entradas del inodo
        struct entrada buffEntradas[entradas_inodo];
        // Leemos todas las entradas
        if(mi_read_f(p_inodo,buffEntradas,0,sizeof(struct entrada) * entradas_inodo) == -1){
            fprintf(stderr,"Error al leer el fichero.\n");
            mi_signalSem(); // Señal semáforo
            return -1;
        }
        int i = 0; // Iterador
        // Vamos imprimiendo todas las entradas del directorio
        for(; i < entradas_inodo; i++){
            if(leer_inodo(buffEntradas[i].ninodo,&inodo) == -1){
                fprintf(stderr,"Error al intentar leer el inodo.\n");
                mi_signalSem(); // Señal semáforo
                return -1;            
            }
            // Función auxiliar que imprime las entradas del directorio que hemos almacenado en el buffer
            aux_informacion_inodo_dir(buffer,inodo,buffEntradas[i]);
        }
        // Finalmente imprimimos todo por pantalla:
        printf(MAGENTA_T"Total: %d\n",i);
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("---------------------------------------------------------------\n");
        printf("%s\n"COLOR_RESET,buffer);
    // Miramos el caso del fichero, donde únicamente listaremos la información del fichero en sí mismo
    }else{
        // Leemos el inodo asociado al directorio padre
        struct inodo inodo_padre;
        if(leer_inodo(p_inodo_dir,&inodo_padre) == -1){
            fprintf(stderr,"Error al intentar leer inodo.\n");
            mi_signalSem(); // Señal semáforo
            return -1;
        }
        // Cálculamos cuántas entradas tiene el inodo padre
        int entradas_inodo = inodo_padre.tamEnBytesLog / sizeof(struct entrada);
        // Buffer auxiliar que almacenará todas las entradas del inodo padre
        struct entrada buffEntradas[entradas_inodo];
        // Leemos todas las entradas del directorio padre
        if(mi_read_f(p_inodo_dir,buffEntradas,0,sizeof(struct entrada) * entradas_inodo) == -1){
            fprintf(stderr,"Error al leer el fichero.\n");
            mi_signalSem(); // Señal semáforo
            return -1;
        }
        // Sabemos que el número de entrada donde se encuentra el fichero es en p_entrada
        // ( Lo sabemos gracias a la función buscar_entrada )
        aux_informacion_inodo_dir(buffer,inodo,buffEntradas[p_entrada]);
        // Finalmente imprimimos todo por pantalla:
        printf(CYAN_T"Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("---------------------------------------------------------------\n");
        printf("%s\n"COLOR_RESET,buffer); 
    }
    mi_signalSem(); // Señal semáforo
    return 0;
}

void aux_informacion_inodo_dir(char *buffer, struct inodo inodo, struct entrada entrada){
    char tmp[80];
    char tamanoBytesLog[80];
    // Incorporamos información sobre el tipo del inodo
    strcat(buffer,(char *)&inodo.tipo);
    strcat(buffer,"\t");
    // Incorporamos información sobres los permisos en el buffer
    if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
    if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
    if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
    strcat(buffer,"\t");
    // Incorporamos información acerca del tiempo en el buffer
    struct tm *tm; 
    tm = localtime(&inodo.mtime);
    sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(buffer,tmp);
    strcat(buffer,"\t");
    // Incorporamos también información del tamaño en bytes del inodo y el nombre de la entrada
    sprintf(tamanoBytesLog,"%d",inodo.tamEnBytesLog);
    strcat(buffer,tamanoBytesLog);
    strcat(buffer,"\t");
    strcat(buffer,entrada.nombre);
    strcat(buffer,"\n"); // Salto de linea 
}

int mi_chmod(const char *camino, unsigned char permisos){
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    //Inicialización de las variables que utilizaremos en extraer camino
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }    
    // Si hemos llegado hasta aquí es que ha salido bien todo
    // así que llamamos a chmod de la capa de ficheros para cambiar permisos
    if(mi_chmod_f(p_inodo,permisos) == -1){
        fprintf(stderr,"Error al intentar cambiar permisos en la capa de ficheros.\n");
        return -1;
    }
    return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat){
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    //Inicialización de las variables que utilizaremos en extraer camino
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        return errores;
    }  
    // Metemos en p_stat los datos del inodo pasado por parámetro
    mi_stat_f(p_inodo,p_stat);
    // Mostramos el número de inodo
    printf("Nº de inodo:%d\n",p_inodo);
    return 0;
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
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
    int iterador = 0;
    // Realizamos una búsqueda de todas las entradas que tenemos en la cache de escritura
    for(;iterador < entradasCache && strcmp(UltimaEntradaEscritura[iterador].camino, camino) != 0; iterador++){}
    // Si lo hemos encontrado, entonces simplemente le asignamos ese inodo al que estamos buscando
    if(strcmp(UltimaEntradaEscritura[iterador].camino, camino) == 0){
        //fprintf(stderr,"[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n");
        //Actualizamos el tiempo asociado a ese camino
        p_inodo = UltimaEntradaEscritura[iterador].p_inodo;
        UltimaEntradaEscritura[iterador].tiempo = time(NULL);
    }else{ // Sino, tendremos que utilizar la función de buscar entrada
        //fprintf(stderr,"[mi_write() → Actualizamos la caché de escritura]\n");
        if(elementosBufferEscritura < entradasCache - 1){
            iterador = elementosBufferEscritura;
            elementosBufferEscritura++;
        }else{
            iterador = 0;
            int posInodo = 0;
            time_t masAntiguo;
            masAntiguo = UltimaEntradaEscritura[posInodo].tiempo;
            iterador = posInodo;
            posInodo++;
            // Realizamos una búsqueda de todas las entradas que tenemos en la cache de escritura
            while(posInodo < entradasCache){
                // inodoAux.ctime > masAntiguo
                if(difftime(UltimaEntradaEscritura[posInodo].tiempo, masAntiguo) < 0){
                    masAntiguo = UltimaEntradaEscritura[posInodo].tiempo;
                    iterador = posInodo;
                }
                posInodo++;
            }           
        }
        // Y hacemos ahora el guardado del camino antes de utilizar la función de guardar entrada
        strcpy(UltimaEntradaEscritura[iterador].camino,camino);
        // Ahora ya sí podemos ir desmenuzando el camino
        int error = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
        // Controlamos posible error
        if(error < 0){
            // Mostramos si ha habido algún error de búsqueda
            mostrar_error_buscar_entrada(error);
            return error;
        }
        // Le asignamos el p_inodo correspondiente a ese camino
        UltimaEntradaEscritura[iterador].p_inodo = p_inodo;
        // Actualizamos el tiempo
        UltimaEntradaEscritura[iterador].tiempo = time(NULL);
    }
    // Finalmente, escribimos en el inodo correspondiente del camino pasado como parámetro
    int bytesEscritos = mi_write_f(p_inodo,buf,offset,nbytes);
    // Revisamos posible error
    if(bytesEscritos == -1){ return -1;}
    // Devolvemos la cantidad de bytes que se han escrito en el fichero
    return bytesEscritos;
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
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
    int iterador = 0;
    // Realizamos una búsqueda de todas las entradas que tenemos en la cache de escritura
    for(;iterador < entradasCache && strcmp(UltimaEntradaLectura[iterador].camino, camino) != 0; iterador++){}
    // Si lo hemos encontrado, entonces simplemente le asignamos ese inodo al que estamos buscando
    if(strcmp(UltimaEntradaLectura[iterador].camino, camino) == 0){
        //fprintf(stderr,"\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n");
        p_inodo = UltimaEntradaLectura[iterador].p_inodo;
        UltimaEntradaLectura[iterador].tiempo = time(NULL);
    }else{ // Sino, tendremos que utilizar la función de buscar entrada
        // Antes de nada, actualizamos el camino de la cache según la estrategia FIFO
        //fprintf(stderr,"\n[mi_read() → Actualizamos la caché de lectura]\n");
        if(elementosBufferLectura < entradasCache - 1){
            iterador = elementosBufferLectura;
            elementosBufferLectura++;
        }else{
            iterador = 0;
            int posInodo = 0;
            time_t masAntiguo;
            masAntiguo = UltimaEntradaLectura[posInodo].tiempo;
            iterador = posInodo;
            posInodo++;
            // Realizamos una búsqueda de todas las entradas que tenemos en la cache de escritura
            while(posInodo < entradasCache){
                // inodoAux.ctime > masAntiguo
                if(difftime(UltimaEntradaLectura[posInodo].tiempo, masAntiguo) < 0){
                    masAntiguo = UltimaEntradaLectura[posInodo].tiempo;
                    iterador = posInodo;
                }
                posInodo++;
            }            
        }
        // Y hacemos ahora el guardado del camino antes de utilizar la función de guardar entrada
        strcpy(UltimaEntradaLectura[iterador].camino,camino);
        // Ahora ya sí podemos ir desmenuzando el camino
        int error = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
        // Controlamos posible error
        if(error < 0){
            // Mostramos si ha habido algún error de búsqueda
            mostrar_error_buscar_entrada(error);
            return error;
        }
        // Le asignamos el p_inodo al camino correspondiente
        UltimaEntradaLectura[iterador].p_inodo = p_inodo;
        // Actualizamos el tiempo
        UltimaEntradaLectura[iterador].tiempo = time(NULL);
    }
    // Finalmente, leemos en el inodo correspondiente del camino pasado como parámetro
    int bytesLeidos = mi_read_f(p_inodo,buf,offset,nbytes);
    // Revisamos posible error
    if(bytesLeidos == -1){ return -1;}
    // Devolvemos la cantidad de bytes que se han escrito en el fichero
    return bytesLeidos;
}

int mi_link(const char *camino1, const char *camino2){
    mi_waitSem(); // Wait del semáforo
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    }
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    // Variables del camino 1
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(camino1,&p_inodo_dir,&p_inodo1,&p_entrada1,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        mi_signalSem(); // Señal del semáforo
        return errores;
    }  
    struct inodo inodo;
    // Leemos el inodo asociado a la ultima entrada de camino1
    if(leer_inodo(p_inodo1,&inodo) == -1){
        fprintf(stderr,"Error al leer inodo.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    }
    // Error correspondiente a los permisos de lectura
    if((inodo.permisos & 4) != 4){
        // Imprimimos fallo
        fprintf(stderr, "No hay permisos de lectura.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;        
    }
    
    // Variables del camino 2
    p_inodo_dir = SB.posInodoRaiz; // Volvemos a poner el inodo asociado al padre del directorio a 0
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    // Reservar es igual a 1 ya que en caso de que no exista, hay que crearla con permisos a 6
    errores = buscar_entrada(camino2,&p_inodo_dir,&p_inodo2,&p_entrada2,1,6);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        mi_signalSem(); // Señal del semáforo
        return errores;
    } 
    struct entrada entradaCamino2;
    // Leemos la entrada correspondiente a camino2
    if(mi_read_f(p_inodo_dir,&entradaCamino2,p_entrada2*sizeof(struct entrada),sizeof(struct entrada) ) == -1){
        fprintf(stderr,"Error al leer la entrada de camino1.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    } 
    // Creamos el enlace, ahora el inodo de la entrada de camino2 es el mismo que el de camino1
    entradaCamino2.ninodo = p_inodo1;
    // Escribimos la entrada correspondiente a camino2
    if(mi_write_f(p_inodo_dir,&entradaCamino2,p_entrada2*sizeof(struct entrada),sizeof(struct entrada) ) == -1){
        fprintf(stderr,"Error al escribir la entrada de camino1.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    } 
    // Liberamos el inodo que estaba asociado a la entrada de camino2
    if(liberar_inodo(p_inodo2) == -1){
        fprintf(stderr,"Error al liberar el inodo de la entrada de camino2.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    } 
    // Incrementamos la cantidad de links que tiene el inodo de la entrada de camino1
    inodo.nlinks++;
    // Actualizamos el ctime del inodo ya que hemos modificado el inodo
    inodo.ctime = time(NULL);
    // Escribimos el inodo que estaba asociado a la entrada de camino1
    if(escribir_inodo(p_inodo1,inodo) == -1){
        fprintf(stderr,"Error al escribir el inodo de la entrada de camino1.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    } 
    mi_signalSem(); // Señal del semáforo
    return 0;
}

int mi_unlink(const char *camino){
    mi_waitSem(); // Wait del semáforo
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    }
    // Variables de los inodos para buscar entrada
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    // Reservar es igual a 0 porque solo queremos consultar
    int errores = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,0);
    // Miramos si ha habido algún error
    if(errores < 0){
        // Mostramos si ha habido algún error de búsqueda
        mostrar_error_buscar_entrada(errores);
        mi_signalSem(); // Señal del semáforo
        return errores;
    }
    // Leemos el inodo asociado a la entrada del camino
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al leer inodo.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;       
    }
    // Miramos si se trata de un directorio y no está vacio, ya que en ese caso no se podrá borrar
    if(camino[strlen(camino)-1] == '/' && inodo.tamEnBytesLog > 0) {
        fprintf(stderr,"Error: el directorio %s no está vacío.\n",camino);
        mi_signalSem(); // Señal del semáforo
        return -1;
    }
    struct inodo inodo_dir;
    // Ahora leemos el inodo referente al directorio padre de esa entrada
    if(leer_inodo(p_inodo_dir,&inodo_dir) == -1){
        fprintf(stderr,"Error al leer inodo.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;       
    }           
    // Calculamos cuantas entradas tiene el inodo del direoctorio padre
    int numEntradasDirPadre = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    int numUltimaEntrada = numEntradasDirPadre - 1;
    // Miramos si la entrada a eliminar no es la última de todas
    if(p_entrada != numUltimaEntrada){ 
        // Leemos la última entrada
        struct entrada ultimaEntrada;
        if(mi_read_f(p_inodo_dir,&ultimaEntrada,numUltimaEntrada * sizeof(struct entrada),sizeof(struct entrada)) == -1){
            fprintf(stderr,"Error al leer la entrada del inodo.\n");
            mi_signalSem(); // Señal del semáforo
            return -1;
        }
        // Ahora colocamos la última entrada en la posición de la que queremos eliminar
        if(mi_write_f(p_inodo_dir,&ultimaEntrada,p_entrada * sizeof(struct entrada),sizeof(struct entrada)) == -1){
            fprintf(stderr,"Error al escribir en la entrada del inodo.\n");
            mi_signalSem(); // Señal del semáforo
            return -1;
        }
    }
    // Truncamos el inodo quitando el tamaño de la entrada eliminada
    if(mi_truncar_f(p_inodo_dir,inodo_dir.tamEnBytesLog - sizeof(struct entrada)) == -1){
        fprintf(stderr,"Error al truncar el inodo.\n");
        mi_signalSem(); // Señal del semáforo
        return -1;
    }
    // Decrementamos el número de enlaces del inodo de la entrada eliminada  
    inodo.nlinks--;
    // Miramos si quedan más enlaces o no
    if(inodo.nlinks == 0){
        // Si no quedan más links, liberamos el inodo
        if(liberar_inodo(p_inodo) == -1){
            fprintf(stderr,"Error al liberar inodo.\n");
            mi_signalSem(); // Señal del semáforo
            return -1;
        }
    }else{ // Si quedan más links asociados a ese inodo
        inodo.ctime = time(NULL); // Actualizamos el ctime ya que hemos modificado el inodo
        // Finalmente, escribimos el inodo
        if(escribir_inodo(p_inodo,inodo) == -1){
            fprintf(stderr,"Error al escribir inodo.\n");
            mi_signalSem(); // Señal del semáforo
            return -1;
        }        
    }
    mi_signalSem(); // Señal del semáforo
    return 0;
}










