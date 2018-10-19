//Juan Vicente Guillen Casas
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mitar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int copiados = 0;
	int lectura;
 
	if(origin == NULL) return -1; //Si no hay archivo origen, no se puede hacer nada.
	
	lectura = getc(origin);
	while(copiados < nBytes && lectura != EOF){		
		if (putc(lectura,destination) == EOF) return -1; // Comprobar que no haya fallo en la escritura del archivo destination.
		copiados += 1;
		if(copiados < nBytes) lectura = getc(origin); // Si no has llegado al limite de Bytes para copiar, sigues leyendo.
	}
	
	return copiados; //Devolvemos el numero de bytes copiados tal y como nos piden.
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{

int tamanoCadena = 0;
char aux;
char * resultado;
	
	aux = getc(file);
	if(aux == EOF || aux == '\0') return NULL; // No permitirems archivos en blanco.
	
	while(aux != '\0'){	
		tamanoCadena += 1;	
		aux = getc(file);
		if(aux == EOF) return NULL; //Si getc nos devuelve un eof significa que ha habido un fallo, devolvemos null
	}
	
	fseek(file, -(tamanoCadena + 1), SEEK_CUR); //Volvemos el cursor donde estaba al principio.

	resultado = malloc(sizeof(char)*(tamanoCadena + 1));
	int count = 0;
	while(count < tamanoCadena + 1){
		resultado[count] = getc(file); //Asignamos a resultado los valores que vamos tomando del archivo.
		count++;
	}
	
	return resultado;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	int error;
	stHeaderEntry * resultado;
	
	error = fread(nFiles,sizeof(int),1,tarFile); // Asignamos en nFiles el primer int que leyamos en el tarfile.
	if(error == 0) return NULL;	//Si encontramos un error a la hora de leer en el tarfile salimos devolviendo null.
	resultado = malloc(sizeof(stHeaderEntry) * (*nFiles));
	
	int a = 0;	
	while(a < *nFiles){	
		resultado[a].name = loadstr(tarFile); //Vamos cargando el nombre del archivo en la posicion a.
		if(resultado[a].name == NULL) return NULL;	//Si no carga nada devolvemos null porque hay un fallo.	
		fread(&resultado[a].size, sizeof(unsigned int), 1, tarFile); // cargamos el tamaño del archivo en la posicion a.
		a++;
	}
	
	return resultado;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	stHeaderEntry * header;
	int totalBytesCabecera = sizeof(int); // Como minimo tiene el tamaño del int que nos dice el numero de archivos que hay.
	
	header = malloc(sizeof(stHeaderEntry)* nFiles);
	totalBytesCabecera +=  nFiles * sizeof(unsigned int); //Sumamos el tamaño de los int que indican el tamaño de sus respectivos archivos.
	
	int a = 0;
	while(a < nFiles){
		totalBytesCabecera += strlen(fileNames[a]) + 1; // Sumamos el tamaño del nombre de los archivos. La posicion en blanco tambien.
		a++;
	}
	
	FILE *salida;
	
	salida = fopen(tarName, "w");
	fseek(salida,totalBytesCabecera ,SEEK_SET); // Dejamos el hueco de cabecera, para luego rellenarlo.
	
	int b = 0;
	
	while( b < nFiles){
		FILE *entrada = fopen(fileNames[b], "r");
		if (entrada == NULL) return EXIT_FAILURE; // Si no se abre correctamente, fallo.
		
		int bytesCopiados = copynFile(entrada,salida, INT_MAX);
		if(bytesCopiados == -1 ) return EXIT_FAILURE; //Si no se pudo realizar la copia correctamente, fallo.
				
		header[b].name = malloc(sizeof(fileNames[b]) + 1);
		strcpy(header[b].name,fileNames[b]); //Función que copia strings y nos ahorra un mogollón de pasos.
		header[b].size = bytesCopiados;
		
		if(fclose(entrada) == EOF) return EXIT_FAILURE; // Si no nos puede cerrar entramos en un fallo.
		b++; 
	}
	
	//Volvemos a la cabecera a rellenarla con el numero de archivos, y sus respectivas duplas(nombre,tamaño).
	
	fseek(salida,0,SEEK_SET);
	fwrite(&nFiles,sizeof(int),1,salida);//Escribimos el numero de archivos que hay.
	
	int c = 0;
	
	while(c < nFiles){ //Escribimos las duplas del header, primero el nombre y luego el tamaño.
		fwrite(header[c].name, strlen(header[c].name) + 1, 1 , salida);	
		fwrite(&header[c].size, sizeof(unsigned int),1,salida);	 
		c++;
	}
	
	int d = 0;
	
	// liberamos la memoria y terminamos.
	
	while(d < nFiles){ // Primero los string de header.
		free(header[d].name);
		d++;
	}
	
	//finalmente header.
	
	free(header);
	
	//cerramos salida.
	
	fclose(salida);
	
	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	FILE *entrada;
	entrada = fopen(tarName,"r");	
	if(entrada == NULL) return EXIT_FAILURE;
	
	
	int nArchiv;		
	stHeaderEntry * header = readHeader(entrada,&nArchiv);	//Leemos la cabecera del archivo mtar y sacamos nuestro array de duplas header.
	if (header == NULL ) return EXIT_FAILURE;
	
	int a = 0;
	
	while(a < nArchiv){
		FILE *salida =  fopen(header[a].name,"w"); //Abrimos el archivo con el nombre obtenido de nuestro header.
		if(salida == NULL) return EXIT_FAILURE;
		
		int bytesCopiados = copynFile(entrada,salida, header[a].size); //Copiamos los bytes del archivo mtar al archivo destino, ayudandonos del tamaño obtenido en el header.
		if(bytesCopiados == -1 ) return EXIT_FAILURE; //Si no se pudo realizar la copia correctamente, fallo.
		
		if(fclose(salida) == EOF) return EXIT_FAILURE;
		a++;
	}

	int b = 0;
	
	// liberamos la memoria y terminamos.
	
	while(b < nArchiv){ // Primero los string de header.
		free(header[b].name);
		b++;
	}
	
	//finalmente header.
	
	free(header);
	
	//cerramos entrada.
	
	fclose(entrada);
	
	return EXIT_SUCCESS;
}

int
listTar(char tarName[]){
  FILE *entrada;
  entrada = fopen(tarName,"r");	
  if(entrada == NULL) return EXIT_FAILURE;

	int nArchiv;		
	stHeaderEntry * header = readHeader(entrada,&nArchiv);	//Leemos la cabecera del archivo mtar y sacamos nuestro array de duplas header.
	if (header == NULL ) return EXIT_FAILURE;
	
	int a = 0;
	
 	while(a < nArchiv){
	printf("%s " ,header[a].name);	
	a++;
	}
   

	int b = 0;
	
	// liberamos la memoria y terminamos.
	
	while(b < nArchiv){ // Primero los string de header.
		free(header[b].name);
		b++;
	}
	
	//finalmente header.
	
	free(header);
	
	//cerramos entrada.
	
	fclose(entrada);
	
	return EXIT_SUCCESS;
}

int
appendTar(int nFiles,char *fileNames[], char tarName[],char tarNameSalida[]){
 FILE *entrada;
  entrada = fopen(tarName,"r");	
  if(entrada == NULL) return EXIT_FAILURE;

int nArchivOld;
int totalBytesCabeceraN = sizeof(int), totalBytesCabeceraV = sizeof(int);
stHeaderEntry * headerViejo = readHeader(entrada,&nArchivOld);  
stHeaderEntry * headerNuevo; //Ahora podemos sacar el hueco de la nueva cadecera.
int nFilesN = nFiles + nArchivOld;
headerNuevo = malloc(sizeof(stHeaderEntry)* nFilesN);
totalBytesCabeceraN +=  nFilesN * sizeof(unsigned int);
totalBytesCabeceraV +=  nArchivOld * sizeof(unsigned int);

int a = 0;
while(a < nFilesN){
	if(a < nArchivOld){
	 totalBytesCabeceraN += strlen(headerViejo[a].name) + 1; //Primero los de la cabezera vieja luego los otros.
	 totalBytesCabeceraV += strlen(headerViejo[a].name) + 1;
	}
	else totalBytesCabeceraN += strlen(fileNames[a - nArchivOld]) + 1; // Sumamos el tamaño del nombre de los archivos. La posicion en blanco tambien.
	a++;
}

fseek(entrada, totalBytesCabeceraV, SEEK_SET);

int b = 0;
FILE *salida = fopen(tarNameSalida, "w");
if (salida == NULL) return EXIT_FAILURE; // Si no se abre correctamente, fallo.
fseek(salida, totalBytesCabeceraN, SEEK_SET);


while( b < nFilesN){
	if(b < nArchivOld){
		int bytesCopiados = copynFile(entrada,salida, headerViejo[b].size);
		if(bytesCopiados == -1 ) return EXIT_FAILURE; //Si no se pudo realizar la copia correctamente, fallo.
				
		headerNuevo[b].name = malloc(sizeof(headerViejo[b].name) + 1);
		strcpy(headerNuevo[b].name,headerViejo[b].name); //Función que copia strings y nos ahorra un mogollón de pasos.
		headerNuevo[b].size = bytesCopiados;

	}
	else{
	FILE * nuevo = fopen(fileNames[b - nArchivOld],"r");
	int bytesCopiados = copynFile(nuevo,salida, INT_MAX);
	if(bytesCopiados == -1 ) return EXIT_FAILURE; //Si no se pudo realizar la copia correctamente, fallo.
	
	headerNuevo[b].name = malloc(sizeof(fileNames[b - nArchivOld]) + 1);
	strcpy(headerNuevo[b].name,fileNames[b - nArchivOld]); //Función que copia strings y nos ahorra un mogollón de pasos.
	headerNuevo[b].size = bytesCopiados;
	if(fclose(nuevo) == EOF) return EXIT_FAILURE;			
	}	
	b++; 
}

if(fclose(entrada) == EOF) return EXIT_FAILURE;

fseek(salida,0,SEEK_SET);
fwrite(&nFilesN,sizeof(int),1,salida);//Escribimos el numero de archivos que hay.
	
	int c = 0;
	
	while(c < nFilesN){ //Escribimos las duplas del header, primero el nombre y luego el tamaño.
		fwrite(headerNuevo[c].name, strlen(headerNuevo[c].name) + 1, 1 , salida);	
		fwrite(&headerNuevo[c].size, sizeof(unsigned int),1,salida);	 
		c++;
	}
	
	int d = 0;	

	while(d < nArchivOld){ // Primero los string de header.
		free(headerViejo[d].name);
		d++;
	}
	
	//finalmente header.
	
	free(headerViejo);

	int e = 0;

	while(e < nFilesN){ // Primero los string de header.
		free(headerNuevo[e].name);
		e++;
	}

	free(headerNuevo);

fclose(salida);	

	return EXIT_SUCCESS;
}







