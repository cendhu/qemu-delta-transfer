#include "migration/sha256.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#ifndef _HASHING_H
#define _HASHING_H

#define TABLE_SIZE 64
#define HASHSIZE 32

#define PAGE_BITS 8 //within int bounds < 32
//#define HASH_INDEX_MASK ((unsigned int) (1 << PAGE_BITS) - 1)

void hash(unsigned char * buffer, int len, unsigned char * sha256sum);

unsigned int getindex(unsigned char * sha256sum, int table_size);

void print_sha256(unsigned char * sha256sum);

struct table_entry {
  char hash_val[HASHSIZE];
  uint32_t page_num;
  bool is_empty;
};

typedef struct table_entry table_entry;

struct hash_table_t {
  table_entry *table;
  uint32_t size;
};

typedef struct hash_table_t hash_table_t;

table_entry* create_table(uint32_t size);

void insert_entry(table_entry *hash_table, uint32_t table_size, uint32_t index, char *hash, uint32_t page_num);

uint32_t find_entry(table_entry *hash_table, char *hash, uint32_t table_size);

void update_entry(table_entry *hash_table, uint32_t table_size, char *old_hash, char *new_hash,  uint32_t page_num);

void delete_entry(table_entry *hash_table, uint32_t table_size, char *hash);

void print_table(table_entry *hash_table, uint32_t size);
#endif
