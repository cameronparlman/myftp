#include "hash.h"

#define HASHSIZE 32000000 

//K&R hash function version 2
unsigned long hash(char *s){
	unsigned long hashval; 
		
	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31 * hashval;
	return hashval % HASHSIZE;
}
