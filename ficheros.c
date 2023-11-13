/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    int bytesEscritos = 0; 
    // Leemos el inodo indicado por el número de inodo pasado por parámetro
    struct inodo inodo;
    leer_inodo(ninodo,&inodo);
    // Solo podemos escribir en el fichero si el permiso así lo indica
    if((inodo.permisos & 2) == 2){ // Si es así, hay permisos de escritura
        // Ahora necesitamos calcular de qué bloque a qué bloque hay que escribir
        unsigned int primerBL = offset / BLOCKSIZE; // Primer bloque lógico, donde empezará la escritura
        unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // Último bloque lógico, donde acabará la escritura
        // Ahora tenemos que calcular el desplazamiento  en bytes en el primer y último bloque respectivamente
        unsigned int desp1 = offset % BLOCKSIZE;
        unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        unsigned char buf_bloque[BLOCKSIZE];
        // Caso en el que solo hay que escribir un bloque
        if(primerBL == ultimoBL){ 
            // Sección critica para cuando llamamos traducir bloque inodo con reservar a 1
            mi_waitSem(); // Hacemos un wait del semáforo
            // Obtenemos el bloque fisico asociado al ninodo pasado por parámetro
            int nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);
            mi_signalSem(); // Señal semáforo
            // Leemos el bloque entero para no sobreescribir posibles datos
            if(bread(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al leer del fichero.\n");
                return -1;
            }            
            // Escribimos en la posición del bloque que toca el buffer original
            memcpy(buf_bloque + desp1, buf_original,nbytes);           
            // Finalmente escribimos el nuevo buffer en memoria
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al escribir en el fichero.\n");
                return -1;
            }
            // Por ultimo calculamos los bytes escritos
            bytesEscritos = desp2 - desp1 + 1;
            
        }else{ // La escritura afecta a más de un bloque
            // PRIMER BLOQUE LÓGICO
            // Sección critica para cuando llamamos traducir bloque inodo con reservar a 1
            mi_waitSem(); // Hacemos un wait del semáforo
            // Obtenemos el bloque fisico asociado al ninodo pasado por parámetro
            int nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);
            mi_signalSem(); // Señal semáforo
            // Leemos el bloque entero para no sobreescribir posibles datos
            if(bread(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al leer del fichero.\n");
                return -1;
            }              
            // Escribimos en la posición del bloque que toca el buffer original
            memcpy(buf_bloque + desp1, buf_original,BLOCKSIZE - desp1);

            // Finalmente escribimos el nuevo buffer en memoria
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al escribir en el fichero.\n");
                return -1;
            }
            // Calculamos bytes escritos 
            bytesEscritos = BLOCKSIZE - desp1;
            // BLOQUES INTERMEDIOS
            // Iremos iterando para avanzar sobre los bloques en los que escribir
            for(int i = primerBL + 1; i < ultimoBL;i++){
                // Sección critica para cuando llamamos traducir bloque inodo con reservar a 1
                mi_waitSem(); // Hacemos un wait del semáforo
                // Obtenemos el bloque fisico asociado al bloque lógico que corresponda 
                nbfisico = traducir_bloque_inodo(ninodo,i,1);
                mi_signalSem(); // Señal semáforo
                // Escribimos en la posición correspondiente
                if(bwrite(nbfisico,buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1){
                    fprintf(stderr,"Error al escribir en el fichero.\n");
                    return -1;
                }
                // Actualizamos bytes escritos
                bytesEscritos += BLOCKSIZE;
            }
            // ÚLTIMO BLOQUE LÓGICO
            // Sección critica para cuando llamamos traducir bloque inodo con reservar a 1
            mi_waitSem(); // Hacemos un wait del semáforo
            // Obtenemos el bloque fisico asociado al ninodo pasado por parámetro
            nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,1);
            mi_signalSem(); // Señal semáforo
            // Leemos el bloque entero para no sobreescribir posibles datos
            if(bread(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al leer del fichero.\n");
                return -1;
            }
            // Escribimos en la posición del bloque que toca el buffer original
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

            // Finalmente escribimos el nuevo buffer en memoria
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr,"Error al escribir en el fichero.\n");
                return -1;
            }
            // Actualizamos los bytes escritos
            bytesEscritos += desp2 + 1;
        }

    }else{ // No hay permisos de escritura y por tanto no podemos hacer nada
        fprintf(stderr,"Error, sin permiso de escritura.\n");
        return -1;
    }
    // Sección critica para cuando leemos y modificamos atributos del inodo
    mi_waitSem(); // Hacemos un wait del semáforo
    // Finalmente actualizamos la meta información del inodo
    leer_inodo(ninodo,&inodo);
    // Comprobamos si hemos superado el tamaño en bytes del inodo
    if((offset + nbytes) > inodo.tamEnBytesLog){
        // Actualizamos el tamaño en bytes del inodo
        inodo.tamEnBytesLog = offset + nbytes;
        // Actualizamos el ctime ya que hemos modificado datos del inodo
		inodo.ctime = time(NULL);
    }
    // En cualquier caso, actualizamos el mtime ya que hemos modificado los datos( el contenido ) del inodo
    inodo.mtime = time(NULL);
    // Y finalmente salvamos todos los cambios del inodo
    escribir_inodo(ninodo,inodo);
    mi_signalSem(); // Señal semáforo
    return bytesEscritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    int bytesLeidos = 0;
    // Leemos el inodo indicado por el número de inodo pasado por parámetro
    struct inodo inodo;
    leer_inodo(ninodo,&inodo);
    // Solo podemos leer del fichero si el permiso así lo indica
    if((inodo.permisos & 4) == 4){
        if(offset >= inodo.tamEnBytesLog){ // No se ha podido leer ningún byte del fichero
            //fprintf(stderr,"Error, no hemos podido leer ningún byte del fichero.\n");
            return bytesLeidos; // No hemos podido leer ningún byte
        }
        if((offset + nbytes) >= inodo.tamEnBytesLog){ // Pretende leer más allá del EOF
            // Leemos sólo los bytes que podemos desde el offset hasta EOF
            nbytes = inodo.tamEnBytesLog - offset;
        }
        // Ahora necesitamos calcular de qué bloque a qué bloque hay que leer
        unsigned int primerBL = offset / BLOCKSIZE; // Primer bloque lógico, donde empezará la lectura
        unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // Último bloque lógico, donde acabará la lectura
        // Ahora tenemos que calcular el desplazamiento en bytes en el primer y último bloque respectivamente
        unsigned int desp1 = offset % BLOCKSIZE;
        unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        // Obtenemos el bloque fisico asociado al ninodo pasado por parámetro

        unsigned char buf_bloque[BLOCKSIZE];
        if(primerBL == ultimoBL){ // Solo tenemos que leer un bloque 
            int nbfisico = traducir_bloque_inodo(ninodo,primerBL,0); // Ahora reservar es igual a 0
            if( nbfisico != -1){ // Hay que comprobar que sí que haya un bloque físico asignado al bloque lógico
                // Leemos el el bloque 
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr,"Error al leer del fichero.\n");
                    return -1;
                }
                // Copiamos los datos al buffer original
                memcpy(buf_original,buf_bloque + desp1, nbytes);                
            }
            // Acumulamos bytes leidos
            bytesLeidos = nbytes;
        }else{ // Hay varios bloques por leer
            // PRIMER BLOQUE LÓGICO
            int nbfisico = traducir_bloque_inodo(ninodo,primerBL,0); // Ahora reservar es igual a 0
            if( nbfisico != -1){ // Hay que comprobar que sí que haya un bloque físico asignado al bloque lógico
                // Leemos el el bloque 
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr,"Error al leer del fichero.\n");
                    return -1;
                }
                // Copiamos los datos al buffer original
                memcpy(buf_original,buf_bloque + desp1, BLOCKSIZE - desp1);                
            }
            // Acumulamos bytes leidos
            bytesLeidos = BLOCKSIZE - desp1;
            // BLOQUES INTERMEDIOS
            // Iremos iterando para avanzar sobre los bloques en los que escribir
            for(int i = primerBL + 1; i < ultimoBL;i++){
                // Obtenemos el bloque fisico asociado al bloque lógico que corresponda 
                nbfisico = traducir_bloque_inodo(ninodo,i,0); 
                if( nbfisico != -1){ // Hay que comprobar que sí que haya un bloque físico asignado al bloque lógico
                    // Leemos el el bloque 
                    if(bread(nbfisico,buf_bloque) == -1){
                        fprintf(stderr,"Error al leer del fichero.\n");
                        return -1;
                    }
                    // Copiamos los datos al buffer original
                    memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);                
                }
                // Actualizamos bytes leídos
                bytesLeidos += BLOCKSIZE;
            }
            // ÚLTIMO BLOQUE LÓGICO
            // Obtenemos el bloque fisico asociado al ninodo pasado por parámetro
            nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,0); 
            if(nbfisico != -1){
                // Leemos el bloque entero para no sobreescribir posibles datos
                if(bread(nbfisico,buf_bloque) == -1){
                    fprintf(stderr,"Error al leer del fichero.\n");
                    return -1;
                }
                // Escribimos en la posición del bloque que toca el buffer original
                memcpy(buf_original + (nbytes - desp2 - 1),buf_bloque, desp2 + 1);
                
            }
            // Actualizamos los bytes leídos
            bytesLeidos += desp2 + 1;
        }

    }else{ // No hay permisos de lectura y por tanto no podemos hacer nada
        fprintf(stderr,"Error, sin permiso de lectura.\n");
        return -1;
    }
    // Sección critica para cuando leemos y modificamos atributos del inodo
    mi_waitSem(); // Wait del semáforo
    // Finalmente actualizamos el atime del inodo
    leer_inodo(ninodo, &inodo); // Leemos inodo
	inodo.atime = time(NULL); // Actualizamos el último acceso
	escribir_inodo(ninodo, inodo); // Salvamos inodo
    mi_signalSem(); // Señal semáforo
    return bytesLeidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    // Leemos el inodo que corresponde al número de inodo pasado por parámetro
	struct inodo inodo;
	leer_inodo(ninodo,&inodo);
    // Una vez que tenemos el inodo, vamos rellenando los campos de metadátos
	p_stat->tipo = inodo.tipo;      
	p_stat->permisos = inodo.permisos;
	p_stat->atime = inodo.atime;
	p_stat->ctime = inodo.ctime;
	p_stat->mtime = inodo.mtime;
	p_stat->nlinks = inodo.nlinks;
	p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
	p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    // Devolvemos 0 si todo ha salido bien
	return 0;    
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    mi_waitSem(); // Señal semáforo
    // Leemos el inodo que corresponde al número de inodo pasado por parámetro
	struct inodo inodo;
	if(leer_inodo(ninodo,&inodo) == -1){
        fprintf(stderr,"Error al intentar leer el inodo.\n");
        mi_signalSem(); // Señal semáforo
        return -1;
    }
    inodo.permisos = permisos;    // Asignamos el nuevo permiso pasado por parámetro  
    inodo.ctime = time(NULL);     // Actualizamos ya que estamos modificando datos del inodo

    if(escribir_inodo(ninodo,inodo)){ // Y guardamos el inodo
        fprintf(stderr,"Error al intentar escribir el inodo.\n");
        mi_signalSem(); // Señal semáforo
        return -1;
    }
    mi_signalSem(); // Señal semáforo
    return 0; // Devolvemos 0 si todo ha ido bien
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    // Leemos el inodo asociado a ese número de inodo
    if(leer_inodo(ninodo,&inodo) == -1){
        fprintf(stderr,"Error al leer el inodo.\n");
        return -1;
    }
    // Comprobamos que el inodo tenga permisos de lectura
    if((inodo.permisos & 2) == 2){
        // No se puede truncar más allá del tamaño en bytes lógicos del fichero
        if(inodo.tamEnBytesLog < nbytes){
            fprintf(stderr,"Error. No se puede truncar más allá del tamaño lógico del fichero/directorio.\n");
            return -1;
        }
        int primerBL;
        if(nbytes % BLOCKSIZE == 0){
            primerBL =  nbytes / BLOCKSIZE;            
        }else{
            primerBL =  (nbytes / BLOCKSIZE) + 1;
        }
        // Llamamos a la función auxiliar para liberar los bloques asociados al inodo
        // a partir del primer bloque calculado previamente
        int bloquesLiberados = liberar_bloques_inodo(primerBL,&inodo); 
        // Actualizamos los datos del inodo
        inodo.mtime = time(NULL); // Hemos modificado los datos del inodo
        inodo.ctime = time(NULL); // Hemos modificado el inodo
        inodo.tamEnBytesLog = nbytes; // Hemos truncado el inodo
        inodo.numBloquesOcupados -= bloquesLiberados; // Tenemos que restar los bloques liberados
        // Finalmente escribimos el inodo actualizado
        if( escribir_inodo(ninodo,inodo) == -1){
            fprintf(stderr,"Error al intentar escribir el inodo.\n");
            return -1;
        }
        // Devolvemos la cantidad de bloques liberados
        return bloquesLiberados;

    }else{
        fprintf(stderr,"Error, sin permisos de lectura del inodo.\n");
        return -1;
    }
}
