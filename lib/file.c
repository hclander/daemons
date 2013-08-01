#include <stdlib.h>
#include <string.h>
#include "file.h"

// Ejemplos de grabar datos con padding
int fwritePadding(const char *filename, void *buf , size_t buffsize, size_t padsize) {
	FILE *file;
	void *padbuff = calloc(padsize,1);

	file = fopen(filename,"w");   // open for write an truncate

	if (!file)
		return -1;

	memcpy(padbuff,buf,(padsize>buffsize)?buffsize:padsize);
	fwrite(padbuff,1,padsize,file);

	free(padbuff);
	fclose(file);
	return 0;
}

// Misma funcioncion pero pudiendo establecer el byte de relleno
int fwritePaddingEx(const char *filename, void *buf , size_t buffsize, size_t padsize,int pad) {
	FILE * file;
    void *padbuff = malloc(padsize);

    file = fopen(filename,"w");   // open for write an truncate

    if (!file)
   		return -1;

    memset(padbuff,pad,padsize);
    memcpy(padbuff,buf,(padsize>buffsize)?buffsize:padsize);
    fwrite(padbuff,1,padsize,file);
    free(padbuff);

	return 0;
}

int fwritePadding1(const char *filename, void *buf , size_t buffsize, size_t padsize) {

	FILE *file;

	file = fopen(filename,"w");   // open for write an truncate


	if (!file)
			return -1;

	if (padsize > buffsize) {
		void * padbuff = malloc(padsize);
		memset(padbuff,0,padsize);
		memcpy(padbuff,buf,padsize);
		fwrite(padbuff,1,padsize,file);
		free(padbuff);

	} else {
		fwrite(buf,1,padsize,file);
	}

	fclose(file);

	return 0;
}


int fwriteOffset(const char *filename, void *buf, size_t buffsize, long offset) {

	int result = -1;
	FILE *file;
	long len;

	file = fopen(filename,"r+");

	if ( file != NULL ) {

		fseek(file, offset,SEEK_SET);
		if (!feof(file)) {

			fwrite(buf,1,buffsize,file);

		} else {
			// El fichero es mas pequeño que el offset indicado
			// Observacion: Como el fichero está abierto para escritura este error no se va a dar
			// ya que el sistema hará el seek a la posicion rellenando con 0 si sobrepasa el tamaño actual
			// del fichero
			result = -2;
		}


		fclose(file);
	}


	return result;
}

int fwriteOffset1(const char *filename, void *buf, size_t buffsize, long offset, int noOverflow) {

	//TODO:  rehacer la funcion para que no expanda el fichero.

	return 0;
}
