/*
--------   INTEGRANTES DEL GRUPO RICARDO GALLI   ---------
Robert Mikhailovskiy
Odilo Fortes Domínguez
Maribel Crespí Valero
----------------------------------------------------------
*/
// Programa de pruebas
#include "directorios.h"

void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}

int main(int argc, char **argv){   
    struct superbloque SB;
    
    bmount(argv[1]);
    // Leemos el super bloque del disco
    bread(posSB,&SB);

    printf("DATOS DEL SUPERBLOQUE:\n");
    printf("posPrimerBloqueMB = %d\n",SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB =  %d\n",SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n",SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = = %d\n",SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n",SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n",SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n",SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n",SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n",SB.cantInodosLibres);
    printf("totBloques = %d\n",SB.totBloques);
    printf("totInodos =  %d\n",SB.totInodos);
/*
    printf("sizeof struct superbloque: %d\n",(int)sizeof(struct superbloque));
    printf("sizeof struct inodo: %d\n",(int)sizeof(struct inodo));
 //NIVEL 3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int bloqueReservado = reservar_bloque();
    bread(posSB,&SB);
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n",bloqueReservado);
    printf("SB.cantBloquesLibres = %d\n",SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    bread(posSB,&SB);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n",SB.cantBloquesLibres);

    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS:\n");
    
    int bitLeido = leer_bit(posSB);
    int posbyte = posSB / 8; // Posición del byte en MB
    int posbit = posSB % 8; // Posición del bit en MB
    int nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",posSB,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",posSB,bitLeido);
    bitLeido = leer_bit(tamSB);
    posbyte = tamSB / 8; // Posición del byte en MB
    posbit = tamSB % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",tamSB,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",tamSB,bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueMB);
    posbyte = SB.posUltimoBloqueMB / 8; // Posición del byte en MB
    posbit = SB.posUltimoBloqueMB % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",SB.posUltimoBloqueMB,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",SB.posUltimoBloqueMB,bitLeido);
    bitLeido = leer_bit(SB.posPrimerBloqueAI);
    posbyte = SB.posPrimerBloqueAI / 8; // Posición del byte en MB
    posbit = SB.posPrimerBloqueAI % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",SB.posPrimerBloqueAI,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",SB.posPrimerBloqueAI,bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueAI);
    posbyte = SB.posUltimoBloqueAI / 8; // Posición del byte en MB
    posbit = SB.posUltimoBloqueAI % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",SB.posUltimoBloqueAI,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",SB.posUltimoBloqueAI,bitLeido);
    bitLeido = leer_bit(SB.posPrimerBloqueDatos);
    posbyte = SB.posPrimerBloqueDatos / 8; // Posición del byte en MB
    posbit = SB.posPrimerBloqueDatos % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",SB.posPrimerBloqueDatos,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",SB.posPrimerBloqueDatos,bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueDatos);
    posbyte = SB.posUltimoBloqueDatos / 8; // Posición del byte en MB
    posbit = SB.posUltimoBloqueDatos % 8; // Posición del bit en MB
    nbloqueMB = posbyte / BLOCKSIZE; // Calculamos la posición relativa del bloque del bit que nos interesa
    nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB; // Calculamos la posición absoluta del bloque del bit que nos interesa
    printf("[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n",SB.posUltimoBloqueDatos,posbyte,posbit,nbloqueMB,nbloqueabs);
    printf("leer_bit(%d) = %d\n",SB.posUltimoBloqueDatos,bitLeido);
    printf("\nDATOS DEL DIRECTORIO RAIZ:\n");

    struct inodo inodos;
    leer_inodo(0,&inodos);
    char atime[80];
	char mtime[80];
	char ctime[80];
    struct tm *ts;
    printf("tipo: %c\n",inodos.tipo);
    printf("permisos: %d\n",inodos.permisos);
    ts = localtime(&inodos.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodos.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodos.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	fprintf(stdout,"ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n",0,atime,mtime,ctime);
    printf("nlinks: %d\n",inodos.nlinks);
    printf("tamEnBytesLog: %d\n",inodos.tamEnBytesLog);
    printf("numBloqueOcupados: %d\n",inodos.numBloquesOcupados);
    printf("\n");
*/

// NIVEL 4
/*
    int r= reservar_inodo('l',6);

    traducir_bloque_inodo(r,8,1);
    traducir_bloque_inodo(r,204,1);
    traducir_bloque_inodo(r,30004,1);
    traducir_bloque_inodo(r,400004,1);
    traducir_bloque_inodo(r,468750,1);

    struct inodo inodo;
	
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];


	leer_inodo(r, &inodo);
	fprintf(stdout,"tipo: %c \npermisos: %d\n",inodo.tipo,inodo.permisos);
	ts = localtime(&inodo.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&inodo.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	fprintf(stdout,"ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n",r,atime,mtime,ctime);
	fprintf(stdout,"nlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n",inodo.nlinks,inodo.tamEnBytesLog,inodo.numBloquesOcupados);
	bread(posSB,&SB);
	fprintf(stdout,"SB.posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);
*/
// Nivel 7
//Mostrar creación directorios y errores
/*
  mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
  mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
  mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
  mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
  mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
  //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
  mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
  mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
  mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
*/
}




