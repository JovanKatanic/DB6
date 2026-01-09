#ifndef COMAP_H
#define COMAP_H

typedef struct CoHmap CoHmap;

void PrintCoHmap(CoHmap *map);
CoHmap *CreateCoHmap(int initialCapacity, float loadFactor, int keySize);
void *Get(CoHmap *map, void *key);
void Put(CoHmap *map, void *key, void *value);

#endif