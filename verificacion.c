/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Cabecera correspondiente
#include "verificacion.h"

int main(int argc, char **argv){
    // Comprobamos que el número de parámetros coincida con la sintaxis
    char *nombreFichero = "/prueba.dat";
    int longitud_fichero = strlen(nombreFichero);
    if(argc != 3){ 
        // Imprimimos cómo debería ser
        fprintf(stderr,"Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return -1;
    }
    // Montamos el dispositivo con el nombre pasado por parámetro
    if(bmount(argv[1]) == -1){
        fprintf(stderr,"Error en el montaje del dispositivo\n");
        return -1;
    }
    // La ruta vendrá como cuarto parámetro que nos llega por consola
    char *ruta = argv[2];
    // Imprimimos la ruta
    printf("dir_sim: %s\n",ruta);
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
    struct inodo inodo;
    if(leer_inodo(p_inodo,&inodo) == -1){
        fprintf(stderr,"Error al leer inodo.\n");
        return -1;
    }
    // Calculamos el número de entradas que tiene el inodo
    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    // Comprobación para ver si el número de entradas coincide con el número de procesos
    printf("numentradas: %d NUMPROCESOS: %d\n",numEntradas, NUMPROCESOS);
    // Cogemos posible error en caso de que no coincidan
    if(numEntradas != NUMPROCESOS){
        fprintf(stderr,"Error, el número de entradas del directorio de la simulación debería coincidir con el número de procesos.\n");
        return -1;
    }
    // Sacamos la ruta completa del fichero
    int longitud_ruta = strlen(ruta);
    char *informe = "informe.txt";
    int longitud_informe = strlen(informe);
    char *ruta_informe = malloc(longitud_ruta + longitud_informe + 1);
    strcat(ruta_informe,ruta);
    strcat(ruta_informe,informe);
    // Creamos el fichero
    if(mi_creat(ruta_informe, 6) < 0){ 
        fprintf(stderr,"Error al intentar crear un fichero.\n");
        return -1;
    }
    // Buffer auxiliar que almacenará todas las entradas del inodo
    struct entrada buffEntradas[numEntradas];
    // Leemos todas las entradas de una
    if(mi_read(ruta,buffEntradas,0,sizeof(struct entrada) * numEntradas) == -1){
        fprintf(stderr,"Error al leer el fichero.\n");
        return -1;
    }
    struct INFORMACION info_registro;
    struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
    int offsetInforme = 0;
    int longitud_escrituraInforme = 0;
    // Pasamos por cada entrada del directorio
    for(int i = 0; i < numEntradas; i++){
        // Inicializamso todos los campos de la información del registro
        // Primero: las fechas
        info_registro.MayorPosicion.fecha = time(NULL);
        info_registro.MenorPosicion.fecha = time(NULL);
        info_registro.PrimeraEscritura.fecha = time(NULL);
        info_registro.UltimaEscritura.fecha = time(NULL);
        // Segundo: el número de escrituras
        info_registro.MayorPosicion.nEscritura = 0;
        info_registro.MenorPosicion.nEscritura = 0;
        info_registro.PrimeraEscritura.nEscritura = 0;
        info_registro.UltimaEscritura.nEscritura = 0;
        // Tercero: el número de registro
        info_registro.MayorPosicion.nRegistro = 0;
        info_registro.MenorPosicion.nRegistro = 0;
        info_registro.PrimeraEscritura.nRegistro = 0;
        info_registro.UltimaEscritura.nRegistro = 0;
        // Finalmente, inicializamos el número de escrituras de dicho registro a 0
        info_registro.nEscrituras = 0;
        // Cogemos el PID de la ruta del directorio
        char *nombrePid = strchr(buffEntradas[i].nombre,'_');
        info_registro.pid = atoi(++nombrePid);
        int offset = 0; // Inicializamos el offset del mi_read a 0
        // Sacamos la ruta del fichero prueba.dat
        int longitudEntrada = strlen(buffEntradas[i].nombre);
        char *rutaPrueba = malloc(longitud_ruta + longitudEntrada + longitud_fichero +1);
        strcpy(rutaPrueba,ruta);
        strcat(rutaPrueba,buffEntradas[i].nombre);
        strcat(rutaPrueba, nombreFichero);
        // Leemos por varios bloques a la vez
        while(mi_read(rutaPrueba,buffer_escrituras,offset, sizeof(buffer_escrituras)) > 0){
            // Vamos mirando cada escritura que hemos leido
            for(int i = 0; i < cant_registros_buffer_escrituras && info_registro.nEscrituras < 50; i++){
                // Comprobamos si la escritura es válida
                if(buffer_escrituras[i].pid == info_registro.pid){
                    // Si es la primera entonces la guardamos directamente
                    if(info_registro.nEscrituras == 0){
                        // Inicializamos la de menor posición pero también las de primer
                        // y ultima escrituras para tener una referencia por si luego hay que hacer 
                        // comparaciones
                        info_registro.MenorPosicion = buffer_escrituras[i];
                        info_registro.PrimeraEscritura = buffer_escrituras[i];
                        info_registro.UltimaEscritura = buffer_escrituras[i];
                    }else{ // No es la primera escritura
                        // Miramos si la fecha es menor
                        if(difftime(buffer_escrituras[i].fecha,info_registro.PrimeraEscritura.fecha) <= 0){
                            if(buffer_escrituras[i].nEscritura < info_registro.PrimeraEscritura.nEscritura){
                                info_registro.PrimeraEscritura = buffer_escrituras[i]; // Si es así actualizamos
                            }
                         // Miramos si la fecha es mayor
                        }else if(difftime(buffer_escrituras[i].fecha,info_registro.UltimaEscritura.fecha) >= 0){
                            if(buffer_escrituras[i].nEscritura > info_registro.UltimaEscritura.nEscritura){
                                info_registro.UltimaEscritura = buffer_escrituras[i]; // Si es así actualizamos
                            }
                        }
                    }
                    // La última escritura será la última realizada
                    info_registro.MayorPosicion = buffer_escrituras[i];
                    // Aumentamos el numero de escrituras validadas
                    info_registro.nEscrituras++;
                }
            }
            // Aumentamos el offset de la lectura
            offset += sizeof(buffer_escrituras);
            // Limpiamos buffer por si acaso
            memset(buffer_escrituras,0,sizeof(struct REGISTRO)*cant_registros_buffer_escrituras);           
        }
        // Finalmente, copiaremos toda esta información en un buffer que luego escribiremos en un fichero
        char bufferInforme[2000];
        char tmp[80];
        char tmp2[80];
        char tmp3[80];
        char tmp4[80];
        // Hacemos la conversión del tiempo para que se pueda ver de manera más visual
        struct tm *tm; 
        tm = localtime(&info_registro.PrimeraEscritura.fecha);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        tm = localtime(&info_registro.UltimaEscritura.fecha);
        sprintf(tmp2, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        tm = localtime(&info_registro.MenorPosicion.fecha);
        sprintf(tmp3, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        tm = localtime(&info_registro.MayorPosicion.fecha);
        sprintf(tmp4, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        // Finalmente lo copiamos todo al buffer
        sprintf(bufferInforme,"\nPID: %d\nNúmero de escrituras %d\nPrimera escritura:\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMenor Posición:\t\t%d\t%d\t%s\nMayor Posición:\t\t%d\t%d\t%s\n\n"
        ,info_registro.pid, info_registro.nEscrituras,
        info_registro.PrimeraEscritura.nEscritura, info_registro.PrimeraEscritura.nRegistro, tmp,
        info_registro.UltimaEscritura.nEscritura, info_registro.UltimaEscritura.nRegistro, tmp2,
        info_registro.MenorPosicion.nEscritura, info_registro.MenorPosicion.nRegistro, tmp3,
        info_registro.MayorPosicion.nEscritura, info_registro.MayorPosicion.nRegistro, tmp4);  
        // Ahora escribimos el informe en el fichero
        longitud_escrituraInforme = (int)strlen(bufferInforme);
        // Lo escribimos todo en el informe
        if(mi_write(ruta_informe, bufferInforme, offsetInforme, longitud_escrituraInforme) == -1){
            fprintf(stderr,"Error al escribir el informe en el fichero.\n");
            return -1;
        } 
        // Vamos a ir escribiendo los informes uno detras de otro, así que tenemos que tener una variable que vaya
        // guardando lo que ocupaban los anteriores
        offsetInforme += longitud_escrituraInforme;
        // Y cada vez limpiamos el buffer de los informes
        memset(bufferInforme,0,sizeof(bufferInforme));
        // Liberamos espacio de memoria reservado anteriormente
        free(rutaPrueba);
        printf("[%d) 50 escrituras validadas en %s]\n",i,rutaPrueba);
    }
    // Liberamos espacio de memoria reservado anteriormente
    free(ruta_informe);
    // Desmontamos el dispositivo
    if(bumount() == -1){
        fprintf(stderr,"Error al intentar desmontar del dispositivo\n");
        return -1;
    } 
}




