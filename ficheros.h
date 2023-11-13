/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Incluimos la cabecera de ficheros_basico
#include "ficheros_basico.h"
// Estructuras

struct STAT{ // Estructura cuya función es proporcionar los metadatos de los inodos
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)

    time_t atime; // Fecha y hora del último acceso a datos: atime
    time_t mtime; // Fecha y hora de la última modificación de datos: mtime
    time_t ctime; // Fecha y hora de la última modificación del inodo: ctime

   unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
   unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos
   unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos 
};
// Funciones
// Nivel 5
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes); // Escribir fichero
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes); // Leer fichero
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat); // Devuelve toda la metainformación de los inodos
int mi_chmod_f(unsigned int ninodo, unsigned char permisos); // Cambia los permisos del inodo
// Nivel 6
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes); // Trunca un fichero / directorio
