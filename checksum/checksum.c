#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define False 0
#define True 1

#define MAX_filename 100
#define MAX_command 125

void get_md5_source(char* filename) { //Calcula o hash do arquivo no servidor e salva em um arquivo .key (via hashlib)
	char command[MAX_command] = "python md5-source.py ";
	strcat(command, filename);
	system(command);
}

void get_md5_dest(char* filename) { //Calcula o hash do arquivo no cliente e salva em um arquivo .key (via hashlib)
	char command[MAX_command] = "python md5-dest.py ";
	strcat(command, filename);
	system(command);
}

int verify() { //Verifica se os dois arquivos de hash criados são iguais
	
	FILE *keyA, *keyB;
	keyA = fopen("hash_source.key", "r");
	keyB = fopen("hash_dest.key", "r");
	char hashA[32], hashB[32];
	
	//Lendo arquivos .key
	fgets(hashA, 32, keyA);
	fgets(hashB, 32, keyB);
	fclose(keyA);
	fclose(keyB);
	
	if (strcmp(hashA,hashB) == 0) return True;
	return False;
}

int main(void) { //Apenas para teste
	
	//Alocação das strings para nomes dos arquivos
	char *source_filename, *dest_filename;
	source_filename = (char*) malloc(sizeof(char) * MAX_filename);
	dest_filename = (char*) malloc(sizeof(char) * MAX_filename);
	
	source_filename = "arquivo.txt"; //Nome do arquivo no servidor
	dest_filename = "arquivo.txt"; //Nome do arquivo no cliente
	
	get_md5_source(source_filename); //Calcula hash do arquivo no servidor
	get_md5_dest(dest_filename); //Calcula hash do arquivo no cliente
	
	printf("%d\n", verify()); //Verifica se as hashes estão iguais
	
	return 0;
}
