#include "hashmap.h"
#include "inari.h"

/* one at a time hash */
unsigned hash_string(char* str) {
  unsigned len = strlen(str);
  unsigned hash = 0;
  unsigned i;
  for(i = 0; i < len; ++i) {
    hash += str[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

hashmap_t* hashmap_new() {
  hashmap_t *map = malloc(sizeof(hashmap_t));
  map->numkeys = 0;
  map->root = NULL;
  return map;
}

void hashnode_destroy(hashnode_t* node) {
  if(!node) {
    return;
  }
  hashnode_destroy(node->left);
  hashnode_destroy(node->right);

  free(node);
}

void hashmap_destroy(hashmap_t* map) {
  hashnode_destroy(map->root);
  free(map);
}

void hashmap_insert(hashmap_t *map, char* key, void* data) {
  unsigned hash = hash_string(key);

  hashnode_t* node = map->root;
  hashnode_t** ins  = &map->root;

  /* walk down the tree */
  while(node) {
    if(hash < node->hash) {
      ins = &node->left;
      node = node->left;
    } else if (hash > node->hash) {
      ins = &node->right;
      node = node->right;
    } else {
      LOG("Hashmap collision! %s => %d", key, hash);
      return;
    }
  }

  map->numkeys++;
    
  node = malloc(sizeof(hashnode_t));
  node->hash = hash;
  node->data = data;
  node->left = node->right = NULL;

  *ins = node;
}

void* hashmap_get(hashmap_t *map, char* key) {
  unsigned hash = hash_string(key);
  hashnode_t* node = map->root;

  while(node) {
    if(hash < node->hash) {
      node = node->left;
    } else if(hash > node->hash) {
      node = node->right;
    } else {
      return node->data;
    }
  }

  return NULL;
}
