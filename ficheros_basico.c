/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Cabecera
#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    int tamano = ( nbloques / 8 ) / BLOCKSIZE ; // Miramos cuantos bloques necesitamos para el Mapa de bits
    int modulo = ( nbloques / 8 ) % BLOCKSIZE ; // Comprobamos que no nos sobren  bits
    if( modulo != 0) { // En caso de que queden bits libres, los recogeremos en un bloque adicional más
        tamano++; // Sumamos un bloque más a tamaño
    }
    return tamano;
}

int tamAI(unsigned int ninodos){
    int tamAI = ( ninodos * INODOSIZE ) / BLOCKSIZE; // Tamaño de bloques de los Inodos
    int modulo = ( ninodos * INODOSIZE ) % BLOCKSIZE; // Comprobamos que no sobren bits
    if(modulo != 0){ // En caso de que sobren bits, añadiremos un bloque más para cogerlos
        tamAI++; // Sumamos un bloque más a tamaño Inodos
    }
    return tamAI;
}

int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;
    // Inicialización de los bloques del disco:
    // Posición del primer bloque en el mapa de bits
    SB.posPrimerBloqueMB = posSB + tamSB; // 0 + 1 = 1
    // Posición del último bloque del mapa de bits
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    // Posición del primer bloque de los Inodos
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    // Posición del último bloque de los Inodos
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    // Posición del primer bloque de datos
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    // Posición del último bloque de datos
    SB.posUltimoBloqueDatos = nbloques - 1;
    // Otras inicializaciones:
    // Posición del inodo raíz en el array de inodos
    SB.posInodoRaiz = 0;
    // Posición del primer inodo libre en el array de inodos
    SB.posPrimerInodoLibre = 0;
    // Cantidad de bloques libres en el sistema de ficheros
    SB.cantBloquesLibres = nbloques;
    // Cantidad de inodos libres en el array de inodos
    SB.cantInodosLibres = ninodos;
    // Cantidad de bloques total
    SB.totBloques = nbloques; 
    // Cantidad de inodos total
    SB.totInodos = ninodos;
    // Y finalmente escribimos la estructura del bloque en el bloque SB
    if(bwrite(posSB,&SB) == -1){
        fprintf(stderr,"Error al escribir en SB.\n");
        return -1;
    }  
    return 1;
}

int initMB(){
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    // Buffer de 1024 bytes ( 1 bloque )
    unsigned char bufferMB[BLOCKSIZE];

    // Calculamos cuántos bloques estan destinados a metadatos
    int bloquesMetadatos = SB.posPrimerBloqueDatos;
    // Y los restamos de la cantidad de bloques disponibles
    SB.cantBloquesLibres -= bloquesMetadatos;
    // Calculamos los bloques de MB que ocuparán estos metadatos
    int bloquesInit = ( bloquesMetadatos / 8 ) / BLOCKSIZE;
    // Ahora calcularemos cuántos bytes del ultimo bloque estan a 1
    int bytesInit = bloquesMetadatos - (8*BLOCKSIZE*bloquesInit);
    // Calculamos el módulo para saber si es necesario un bloque adicional
    int moduloBloquesInit = ( bloquesMetadatos / 8 ) % BLOCKSIZE;        
    if(moduloBloquesInit != 0){ // En caso de que sobraran bits, sumamos un bloque más
        bloquesInit++;
    }
    // Inicializamos el buffer a todo 1,s para los bloques
    memset(bufferMB,255,BLOCKSIZE);
    // Vamos poniendo a 1 todos los bloques ocupados por metadatos
    for(int i = SB.posPrimerBloqueMB; i < bloquesInit; i++){
        if(bwrite(i,bufferMB) == -1){
            fprintf(stderr,"Error al escribir en el MB.\n");
            return -1;
        }
    }
    // Y ahora ponemos el buffer a 0 de nuevo
    int bytesSobrantes = bytesInit % 8 ;
    memset(bufferMB,0,BLOCKSIZE);
    bytesInit /= 8;
    for(int i = 0; i < bytesInit; i++){
        bufferMB[i] = 255;
    }
    // Finalmente, si quedaban algunos bits por asignar, los tendremos
    // que tratar de la siguiente manera:
    // Primero, colocamos una máscara de todo 1,s en el byte que nos interesa
    unsigned char mascara = 255;
    // Después, calcularemos cuántos desplazamientos de bits tendremos que hacer
    // en ese byte en particular para que quede de la manera correcta
    mascara <<= (8 - bytesSobrantes);
    bufferMB[bytesInit] = mascara;
    // Ahora tendremos que salvar los cambios en el disco
    if(bwrite(bloquesInit,bufferMB) == -1){
        fprintf(stderr,"Error al escribir en el MB.\n");
        return -1;
    }

    // Ponemos el array con todo 0,s
    memset(bufferMB,0,BLOCKSIZE);
    // Inicializamos todos los bloques a 0
    // Acabamos de escribir los 0,s que faltan
    for(int i = bloquesInit + 1; i <= SB.posUltimoBloqueMB;i++){
        if(bwrite(i,bufferMB) == -1){
            fprintf(stderr,"Error al escribir en el MB.\n");
            return -1;
        }   
    }
    // Y actualizamos
    if(bwrite(posSB,&SB) == -1){
        fprintf(stderr,"Error al escribir en SB.\n");
        return -1;
    }    
    return 1;     
}

int initAI(){
    // Variables
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1;
    int salida = 0; // Variable para saber si tenemos que salir antes de tiempo del último bloque de inodos
    for(int i = SB.posPrimerBloqueAI;i <= SB.posUltimoBloqueAI && salida == 0; i++){
        for(int j = 0; j < (BLOCKSIZE / INODOSIZE) && salida == 0;j++){
            inodos[j].tipo = 'l';
            if(contInodos < SB.totInodos){ // Mientras no hayamos llegado al último inodo libre del array
                // Haremos que apunte al siguiente nodo
                inodos[j].punterosDirectos[0] = contInodos++;
            }else{ // Hemos llegado al último nodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                salida = 1;             
            }
        }
        // Escribimos el bloque de inodos en el dispositivo virtual
        if(bwrite(i,inodos) == -1){
            fprintf(stderr,"Error al escribir en el inodo.\n");
            return -1;
        }      
    }
    return 1;
}

int escribir_bit(unsigned int nbloque, unsigned int bit){
    // Variables
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000 en binario
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }

    int posbyte = nbloque / 8; // Posición del byte en MB
    int posbit = nbloque % 8; // Posición del bit en MB
    int nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    if(bread(nbloqueabs,bufferMB) == -1 ){  // Leemos el bloque donde se encuentra el bit que queremos cambiar
        fprintf(stderr,"Error al leer el MB.\n");
        return -1;
    } 
    posbyte %= BLOCKSIZE; // Queremos saber el byte que nos interesa del bloque que acabamos de leer
    mascara >>= posbit; // Para acabar de colocar la máscara
    if(bit == 1){
        bufferMB[posbyte] |= mascara; // Ponemos un 1 en el bit que corresponde
    }else{ // bit  == 0
        bufferMB[posbyte] &= ~mascara; // Ponemos un 0 en el bit que corresponde
    }
    // Guardamos los cambios en el sistema de ficheros
    if( bwrite(nbloqueabs,bufferMB) == -1) {
        fprintf(stderr,"Error al escribir en el MB.\n");
        return -1;
    }
    return nbloqueabs;
}

char leer_bit(unsigned int nbloque){
    // Variables
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char mascara = 128; // 10000000 en binario
    int posbyte = nbloque / 8; // Posición del byte en MB
    int posbit = nbloque % 8; // Posición del bit en MB
    int nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    if(bread(nbloqueabs,bufferMB) == -1){
        fprintf(stderr,"Error al leer el MB.\n");
        return -1;
    }
    posbyte %= BLOCKSIZE; // Queremos saber el byte que nos interesa del bloque que acabamos de leer
    mascara >>= posbit; // Movemos el bit que estaba a 1 en la posición que nos interesa
    mascara &= bufferMB[posbyte]; // Hacemos una AND de la posición del bit
    mascara >>= (7 - posbit); // Desplazamiento para conseguir el valor que nos insteresa
    return mascara;
}

int reservar_bloque(){
    // Necesitamos leer el super bloque para saber si quedan bloques libres en el sistema
    struct superbloque SB;
    if(bread(posSB,&SB) == -1 ){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    // Comprobamos que todavía queden bloques libres para saber si podemos reservar
    if(SB.cantBloquesLibres > 0){
        // Buffer auxiliar donde cada byte será inicializado con un 255
        unsigned char bufferAux[BLOCKSIZE]; 
        memset(bufferAux,255,BLOCKSIZE); // Inicializamos buffer
        // Buffer para ir leyendo los bloques
        unsigned char bufferMB[BLOCKSIZE];
        int posBloqueMB = SB.posPrimerBloqueMB; // Iterador para saber en qué bloque está el bit a 0
        // Localizamos el bloque libre
        // Vamos leyendo los bloques
        while(posBloqueMB <= SB.posUltimoBloqueMB){
            // Vamos leyendo los bloques
            if(bread(posBloqueMB, bufferMB) == -1){
                fprintf(stderr,"Error al leer el MB.\n");
                return -1;
            }
            // Si encontramos el bloque que contenga un 0, salimos
            if(memcmp(bufferMB,bufferAux,BLOCKSIZE) < 0){
                break;
            }
            posBloqueMB++;
        }

        // Al acabar este bucle, ya habremos encontrado el byte del bloque donde está el bit que queremos reservar
        int posbyte = 0;
        // Mientras no nos pasemos del límite
        while(posbyte < BLOCKSIZE){
            // Si encontramos el byte que contiene un 0, salimos
            if(bufferMB[posbyte] < 255){
                break;
            }
            posbyte++;
        }
        // Ahora vamos a buscar el bit a 0 dentro de ese byte
        unsigned char mascara = 128; // 10000000
        int posbit = 0;
        while(bufferMB[posbyte] & mascara){
            bufferMB[posbyte] <<= 1; // desplazamos a la izquierda el bit 0
            posbit++;
        }
        // Finalmente, calculamos el número de bloque que está libre
        int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
        // Ponemos ese bloque a 1 en el mapa de bits
        escribir_bit(nbloque,1);
        // Decrementamos la variable que cuenta los bloques libres
        SB.cantBloquesLibres--;
        // Y actualizamos
        if(bwrite(posSB,&SB) == -1){
            fprintf(stderr,"Error al escribir en el SB.\n");
            return -1;
        }   
        // Reutilizamos el buffer auxiliar para colocarle ahora todo 0,s
        memset(bufferAux,0,BLOCKSIZE);
        // Y lo escribimos en el dispositivo
        if(bwrite(nbloque,bufferAux) == -1){
            fprintf(stderr,"Error al escribir en el MB.\n");
            return -1;
        }          
        return nbloque;
    }else{ // En el caso de que ya no queden ningún bloque libre
        fprintf(stderr,"Error. No queda ningún bloque libre para asignar.\n");
        return -1;
    }
}

int liberar_bloque(unsigned int nbloque){
    // Haremos uso de la función para escribir un bit y lo pondremos a 0 para dejarlo libre
    escribir_bit(nbloque,0);
    // Leemos el super bloque para poder actualizar la cantidad de bloques libres
    struct superbloque SB;
    if(bread(posSB,&SB) == -1 ){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    // Ahora hay un bloque libre más
    SB.cantBloquesLibres++;
    // Y actualizamos
    if(bwrite(posSB,&SB) == -1){
        fprintf(stderr,"Error al escribir en SB.\n");
        return -1;
    }   
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo inodo){   
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // Leemos el superbloque
    if(bread(posSB,&SB) == -1 ){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    
    // Obtenemos el bloque donde se almacena el nodo que queremos modificar del array
    int nbloqueInodo = ( ninodo / (BLOCKSIZE / INODOSIZE) ) + SB.posPrimerBloqueAI;
    // Obtenemos la posición del array respecto a ese bloque
    int posArrayInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    // Leemos el inodo
    if(bread(nbloqueInodo,inodos) == -1 ){
        fprintf(stderr,"Error al leer el inodo.\n");
        return -1;
    }
    // Ahora escribimos el inodo introducido por parámetro
   inodos[posArrayInodo] = inodo;
   // Y finalmente escribimos el inodo actualizado en el bloque que le toca en memoria
   if(bwrite(nbloqueInodo,inodos) == -1){
        fprintf(stderr,"Error al escribir en el inodo.\n");
        return -1;
    }   
    return 0;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    // Leemos el superbloque
    if(bread(posSB,&SB) == -1 ){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    
    // Obtenemos el bloque donde se almacena el nodo que queremos modificar del array
    int nbloqueInodo = ( ninodo / (BLOCKSIZE / INODOSIZE) ) + SB.posPrimerBloqueAI;
    // Obtenemos la posición del array respecto a ese bloque
    int posArrayInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    // Leemos el inodo
    if(bread(nbloqueInodo,&inodos) == -1 ){
        fprintf(stderr,"Error al leer el inodo.\n");
        return -1;
    }
    // Ahora el puntero inodo apunta a la posición del array que queremos leer
    *inodo = inodos[posArrayInodo];
    return 0;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
   struct superbloque SB;
    //Leemos el superbloque
    if(bread(posSB,&SB) == -1 ){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }

    if(SB.cantInodosLibres > 0){
        // Primero utilizaremos esta variable auxiliar que apunte al mismo lugar que el primer inodo libre
        unsigned int posInodoReservado = SB.posPrimerInodoLibre;
        struct inodo inodo;
        // Declaramos variables para saber cuántos punteros Directos e Indirectos hay
        int numPunterosDir = sizeof(inodo.punterosDirectos) / sizeof(inodo.punterosDirectos[0]);
        int numPunterosIndir = sizeof(inodo.punterosIndirectos) / sizeof(inodo.punterosIndirectos[0]);
        // Leemos el inodo de la primera posición libre
        if( leer_inodo(SB.posPrimerInodoLibre, &inodo) == -1 ){
            fprintf(stderr,"Error al intentar leer un inodo.\n");
            return -1;
        }
        // Actualizamos el primerInodoLibre
        SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
        // Inicializamos todos los campos
        inodo.tipo = tipo;
        inodo.permisos = permisos;
        inodo.nlinks = 1;
        inodo.tamEnBytesLog = 0;
        inodo.atime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.mtime = time(NULL);
        inodo.numBloquesOcupados = 0;
        // Recorremos todos los punteros directos para inicializarlos a 0
        for (int i = 0; i < numPunterosDir; i++){
            inodo.punterosDirectos[i] = 0;
        }
        // Recorremos todos los punteros indirectos para inicializarlos a 0
        for (int i = 0; i < numPunterosIndir; i++){
            inodo.punterosIndirectos[i] = 0;
        }
        // Ahora escribiremos el inodo en el sistema
        escribir_inodo(posInodoReservado, inodo);
        // Y restamos un inodo libre
        SB.cantInodosLibres--;

        // Y finalmente escribimos el super bloque actualizado
        if(bwrite(posSB,&SB) == -1){
            fprintf(stderr,"Error al escribir en el fichero.\n");
            return -1;
        }  
        return posInodoReservado;
        

    }else{ // No quedan inodos libres por asignar
        fprintf(stderr,"Error. No queda ningún inodo libre por asignar.\n");
        return -1;
    }

}


int obtener_nRangoBL (struct inodo *inodo,unsigned int nblogico,unsigned int *ptr){
    // Vamos mirando en qué rango está el bloque pasado por parámetro
    if(nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0; // Rango 0
    }else if(nblogico < INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1; // Rango 1
    }else if(nblogico < INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2; // Rango 2
    }else if(nblogico < INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        return 3; // Rango 3
    }else{
        *ptr = 0; // El puntero no apunta a nada
        fprintf(stderr,"Error. Bloque lógico fuera de rango.\n");
        return -1;
    }
}

int obtener_indice (int nblogico, int nivel_punteros){
    if(nblogico < DIRECTOS){ // Ya estamos directamente sobre el índice que queremos devolver
        return nblogico;
    }else if(nblogico < INDIRECTOS0){
        return nblogico - DIRECTOS; // Restamos los bloques que no pertenecen a este rango
    }else if(nblogico < INDIRECTOS1){
        if(nivel_punteros == 2){
            return  (nblogico - INDIRECTOS0) / NPUNTEROS; // Hacemos la división entera con el número de índices
        }else if(nivel_punteros == 1){
            return  (nblogico - INDIRECTOS0) % NPUNTEROS; // Hacemos el módulo para ver el índice del otro nivel
        }
    }else if(nblogico < INDIRECTOS2){
        if(nivel_punteros == 3){
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS) ; // División entera con el número total de índices
        }else if(nivel_punteros == 2){
            return  ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS; // Módulo de la anterior división
        }else if(nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS; // Módulo de la anterior división
        }
    }else{
        fprintf(stderr,"Error al obtener índice.\n");
        return -1;
    }
    return -1;
} 

int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar){
    // Variables
    struct inodo inodo;
    unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];
    // Leemos el inodo correspondiente al número de inodo pasado por parámetro
    if(leer_inodo(ninodo, &inodo) == -1){
        fprintf(stderr,"Error. Fallo al intentar leer inodo.\n");
        return -1;
    }
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL; // El nivel_punteros más alto es el que cuelga del inodo
    while(nivel_punteros > 0){  // Iterar para cada nivel de indirectos
        if(ptr == 0){   // No cuelgan bloques de punteros
            if(reservar == 0){
                //fprintf(stderr,"Error. Lectura de bloque inexistente.\n");
                return -1;  
            }  //reservar bloques punteros y crear enlaces desde inodo hasta datos
            salvar_inodo = 1;
            ptr = reservar_bloque(); //de punteros   
            if( ptr == -1 ){ // Recogemos posible error
                fprintf(stderr,"Error al reservar bloque.\n");
                return -1;
            }      
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL); //fecha actual
            if(nivel_punteros == nRangoBL){
                //el bloque cuelga directamente del inodo
                inodo.punterosIndirectos[nRangoBL - 1] = ptr; // (imprimirlo para test)
                //fprintf(stderr,"[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n", nRangoBL - 1,ptr,ptr,nivel_punteros);
            }else{ //el bloque cuelga de otro bloque de punteros
                buffer[indice] = ptr; 
                if(bwrite(ptr_ant, buffer) == -1){ // Recogemos posible error
                    fprintf(stderr,"Error al escribir en el fichero.\n");
                    return -1;
                }
                //fprintf(stderr,"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n",nivel_punteros +1 , indice, ptr, ptr, nivel_punteros );                 
            }        
        }
        if(bread(ptr,buffer) == -1 ){ // Recogemos posible error
            fprintf(stderr,"Error al leer el buffer.\n");
            return -1;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        if( indice == -1 ){ // Recogemos posible error
            fprintf(stderr,"Error al obtener indice de bloque físico.\n");
            return -1;
        }
        ptr_ant = ptr; //guardamos el puntero
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--; 
    }
    if(ptr == 0){ //no existe bloque de datos
        // Recogemos posible error
        if(reservar == 0){ return -1;}
        salvar_inodo = 1;
        ptr = reservar_bloque(); //de datos
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        if(nRangoBL == 0){
            inodo.punterosDirectos[nblogico] = ptr;
            //fprintf(stderr,"[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n", nblogico, ptr, ptr, nblogico);
        }else{
            buffer[indice] = ptr;
            if(bwrite(ptr_ant, buffer) == -1){ // Recogemos posible error
                fprintf(stderr,"Error al escribir en el fichero.\n");
                return -1;
            }  
            //fprintf(stderr,"[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros + 1, indice, ptr , ptr, nblogico);  
        }        
    }
    if(salvar_inodo == 1){
        if( escribir_inodo(ninodo, inodo) == -1 ){  //sólo si lo hemos actualizado
            fprintf(stderr,"Error al intentar escribir inodo.\n");
            return -1;  
        }  
    }
    return ptr; //nbfisico del bloque de datos
}

int liberar_inodo(unsigned int ninodo){
    struct inodo inodo;
    // Leemos el inodo asociado a ese número de inodo
    if(leer_inodo(ninodo,&inodo) == -1){
        fprintf(stderr,"Error al leer el inodo.\n");
        return -1;
    }
    // Llamamos a la función auxiliar para liberar todos los bloques asociados al inodo
    // El primer parámetro es 0 ya que queremos liberar todos los bloques
    int bloquesLiberados = liberar_bloques_inodo(0,&inodo); 
    if( bloquesLiberados == -1){
        fprintf(stderr,"Error al liberar los bloques del inodo.\n");
        return -1;
    }
    // Restamos la cantidad de bloques liberados
    inodo.numBloquesOcupados -= bloquesLiberados;
    // Ahora el inodo pasa a estar libre
    inodo.tipo='l';
    // Hemos vaciado el inodo y por lo tanto ya no ocupa nada
    inodo.tamEnBytesLog = 0;
    // Ahora toca actualizar la lista enlazada de inodos libres
    struct superbloque SB;
    // Leemos el super bloque del disco
    if(bread(posSB,&SB) == -1){
        fprintf(stderr,"Error al leer el SB.\n");
        return -1;
    }
    // El inodo liberado apunta ahora a la cabeza de la lista
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    // Y la cabeza de la lista apuntará ahora al inodo liberado
    SB.posPrimerInodoLibre = ninodo;
    // Hay que actualizar que ahora hay un inodo libre más
    SB.cantInodosLibres++;
    // Finalmente escribimos el inodo y lo salvamos
    if ( escribir_inodo(ninodo,inodo) == -1 ){
        fprintf(stderr,"Error al intentar escribir un inodo.\n");
        return -1;
    }
    // Conjuntamente también salvamos el superbloque
    if(bwrite(posSB,&SB) == -1){
        fprintf(stderr,"Error al escribir en el SB.\n");
        return -1;
    }  
    // Devolvemos el número de inodo reservado 
    return ninodo;    
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo){
    // Variables
    unsigned int nivel_punteros, indice, ptr, ultimoBL;             // Declaración de variables
    int nRangoBL;                                                   // Declaración de más variables
    unsigned int bloques_punteros [3] [NPUNTEROS];                  // Array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];                       // Array auxiliar para poder hacer comparaciones
    int ptr_nivel [3];                                              // Punteros a bloques de punteros de cada nivel
    int indices[3];                                                 // Indices de cada nivel
    int liberados;                                                  // Nº de bloques liberados
    liberados = 0;
    // Comprobamos si el fichero está vacío
    if(inodo->tamEnBytesLog == 0){ return 0; }
    // Calculamos el ultimo bloque lógico
    if((inodo->tamEnBytesLog % BLOCKSIZE) == 0){
        ultimoBL = ( inodo->tamEnBytesLog / BLOCKSIZE ) - 1;
    }else{
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
    // Inicializamos el buffer auxiliar con todo 0,s
    memset(bufAux_punteros,0,BLOCKSIZE);
    // Y también inicializamos ptr a 0
    ptr = 0;
    // Finalmente imprimimos la información referente al primer y último bloque lógico
    //printf("[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n", primerBL, ultimoBL);
    // Ahora recorreremos todos los bloques lógicos
    for (int nBL = primerBL;nBL <= ultimoBL;nBL++){ 
        // Obtenemos el rango del bloque lógico que estamos tratando
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
        if(nRangoBL < 0){ // Posible error al calcular el rango
            fprintf(stderr,"Error al calcular el rango del inodo.\n");
            return -1;
        }
        nivel_punteros = nRangoBL; //el nivel_punteros +alto cuelga del inodo
        // Mientras cuelguen bloques de punteros
        while (ptr > 0 && nivel_punteros > 0){
            // Calculamos el índice del bloque de punteros
            indice = obtener_indice(nBL,nivel_punteros);   
            if(indice == 0 || nBL == primerBL){
                // Solo leemos del dispositivo si no está ya cargado previamente en un buffer    
                if(bread(ptr,bloques_punteros[nivel_punteros - 1]) == -1){
                    fprintf(stderr,"Error al leer del disco.\n");
                    return -1;
                }
            }
            // Actualizamos los datos
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros -1 ][indice];
            nivel_punteros--;    
        }    
        if(ptr > 0){ // Si existe bloque de datos
            // Liberamos ese bloque
            int nBF = liberar_bloque(ptr);
            if(nBF == -1){ // Comprobamos que no haya habido ningún error
                fprintf(stderr,"Error al intentar liberar bloque.\n");
                return -1; 
            }
            // Ahora habrá un bloque libre más
            liberados++;
            // Imprimimos la información pertinente
            //printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n",nBF,nBL);
            if(nRangoBL == 0){ // Si es un puntero directo
                // Entonces lo vaciamos y lo dejamos a 0
                inodo->punterosDirectos[nBL] = 0;
            }else{ // Es indirecto
                while(nivel_punteros < nRangoBL){
                    // Actualizamos valores
                    indice = indices[nivel_punteros];
                    bloques_punteros [nivel_punteros][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros];
                    if(memcmp(bloques_punteros[nivel_punteros],bufAux_punteros,BLOCKSIZE) == 0){
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        if(liberar_bloque(ptr) == -1){
                            fprintf(stderr,"Error al liberar un bloque.\n");
                            return -1; 
                        }
                        // Subimos un nivel de punteros
                        nivel_punteros++;
                        // Ahora hay un bloque libre más
                        liberados++;
                        // Miramos si el nivel de punteros coincide con el rango del bloque lógico
                        if(nivel_punteros == nRangoBL){
                            // Ponemos un 0 porque lo estamos liberando
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        // Imprimimos los datos pertinentes
                        //printf("[liberar_bloques_inodo()→ liberado BF %d  de punteros de nivel %d correspondiente al BL %d]\n",ptr,nivel_punteros,nBL);                        
                    }else{ // Escribimos en el dispositivo el bloque de punteros modificado
                        //Escribimos en el disco
                        if(bwrite(ptr, bloques_punteros[ nivel_punteros]) == -1){
                            fprintf(stderr,"Error al escribir en el disco.\n");
                            return -1;
                        }
                        // Salimos del bucle ya que no hace falta liberar los bloques superiores                        
                        nivel_punteros = nRangoBL; 
                    }
                }
            }
        }
    }
    // Finalmente, imprimimos cuántos bloques hemos liberado
    //printf("[liberar_bloques_inodo()→ total bloques liberados: %d]\n", liberados);
    //printf("Liberados: %d\n",liberados);
    return liberados;
}

