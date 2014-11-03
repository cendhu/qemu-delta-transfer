#include "migration/sha256.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef _HASHING_H
#define _HASHING_H

#define HASHSIZE 32

void hash(unsigned char * buffer, int len, unsigned char * sha256sum);

unsigned int getindex(unsigned char * sha256sum, int table_size);

void print_sha256(unsigned char * sha256sum);

struct table_entry {
  char hash_val[HASHSIZE];
  int64_t page_addr;
  bool is_empty;
};

typedef struct table_entry table_entry;

struct hash_table_t {
  table_entry *table;
  int size;
};

typedef struct hash_table_t hash_table_t;

table_entry* create_table(int size);

void insert_entry(table_entry *hash_table, int table_size, char *hash, int64_t page_addr);

int find_entry(table_entry *hash_table, char *hash, int table_size);

void update_entry(table_entry *hash_table, int table_size, char *old_hash, char *new_hash,  int64_t page_addr);

int delete_entry(table_entry *hash_table, int table_size, char *hash);

void print_table(table_entry *hash_table, int size);
#endif
