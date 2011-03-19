#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <string.h>
#include <stdlib.h>

/* provides a simple hashmap structure, using 
 * a binary search tree to hold the nodes
 */

typedef struct hashnode {
  unsigned hash;
  void* data;

  struct hashnode* left;
  struct hashnode* right;
} hashnode_t;

typedef struct hashmap {
  unsigned numkeys;

  hashnode_t* root;
} hashmap_t;

hashmap_t* hashmap_new();

void hashmap_destroy(hashmap_t* map);

void hashmap_insert(hashmap_t* map, char* key, void* data);

void* hashmap_get(hashmap_t* map, char* key);

#endif /* _HASHMAP_H_ */
