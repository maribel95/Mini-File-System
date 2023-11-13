/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Última capa de nuestro sistema
// Incluimos cabecera de la capa ficheros
#include "ficheros.h"
#include <stdlib.h> // Tambien incluimos esta librería para poder poner colorines al texto
// Constantes
#define tamnombre 60 //tamaño del nombre de directorio o fichero, en ext2 = 256
#define ERROR_CAMINO_INCORRECTO -1
#define ERROR_PERMISO_LECTURA -2
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -3
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -4
#define ERROR_ENTRADA_YA_EXISTENTE -6
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -7
#define ERROR_PERMISO_ESCRITURA -5
#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) //suponemos un máx de 1000 entradas, aunque debería ser 
#define CYAN_T     "\x1b[36m"
#define MAGENTA_T  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios
// Estructuras
struct entrada {
  char nombre[tamnombre];
  unsigned int ninodo;
};
struct UltimaEntrada{
  char camino [TAMNOMBRE*PROFUNDIDAD];
  int p_inodo;
};
struct cache{
  char camino[TAMNOMBRE*PROFUNDIDAD];
  int p_inodo;
  time_t tiempo;
};


// Funciones
// Nivel 7
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo); // Función que separa el contenido de camino
void mostrar_error_buscar_entrada(int error); // Función auxiliar para encontrar posibles errores en la entrada
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
// Nivel 8
int mi_creat(const char *camino, unsigned char permisos); // Función para crear ficheros o directorios
int mi_dir(const char *camino, char *buffer, char tipo); // Función para listar información
int mi_chmod(const char *camino, unsigned char permisos); // Cambia los permisos del fichero o directorio
int mi_stat(const char *camino, struct STAT *p_stat); // Mostramos el inodo y la información referente 
void aux_informacion_inodo_dir(char *buffer, struct inodo inodo, struct entrada entrada); // Función auxiliar de mi_dir
// Nivel 9
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes); // Función de escritura ya en la capa de directorios
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes); // Función de lectura ya en la capa de directorios
// Nivel 10
int mi_link(const char *camino1, const char *camino2); // Función para enlazar ficheros en un mismo inodo
int mi_unlink(const char *camino); // Función para deshacer enlaces entre fichero


