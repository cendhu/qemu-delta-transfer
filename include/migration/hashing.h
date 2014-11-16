#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "migration/sha256.h"

#ifndef _HASHING_H
#define _HASHING_H

#define HASHSIZE 32

#define TARGET_PAGE_BITS 12
#define TARGET_PAGE_SIZE (1 << TARGET_PAGE_BITS)

#define SUB_PAGE_SIZE (TARGET_PAGE_SIZE >> 2)
#define PAGE_OFFSET_MASK (TARGET_PAGE_SIZE-1)

void hash(unsigned char * buffer, int len, unsigned char * sha256sum);

unsigned int getindex(unsigned char * sha256sum, int table_size);

void print_sha256(unsigned char * sha256sum);

struct table_entry {
  char hash_val[HASHSIZE];
  int64_t page_addr;
  bool is_empty;
};

typedef struct table_entry table_entry;

struct table_entry_node {
	int64_t addr;
	char hash_val[HASHSIZE];
	bool is_empty;
	struct table_entry_node *next;
};

typedef struct table_entry_node table_entry_node;

struct hash_table_chained {
	int size;
	table_entry_node *table;
};

typedef struct hash_table_chained hash_table_chained;

struct hash_table_t {
  table_entry *table;
  int size;
};

typedef struct hash_table_t hash_table_t;

table_entry* create_table(int size);

table_entry_node* create_table_chained(int size);

void insert_entry_c(table_entry_node *hash_table, int table_size, char *hash, int64_t page_addr);

int delete_entry_c(table_entry_node *hash_table, int table_size, char *hash, int64_t page_addr);

table_entry_node *get_list_c(table_entry_node *hash_table, char *hash, int table_size);

table_entry_node *find_next_c(table_entry_node *n, char *hash, int64_t page_addr);

void insert_entry(table_entry *hash_table, int table_size, char *hash, int64_t page_addr);

int find_entry(table_entry *hash_table, char *hash, int table_size);

void update_entry(table_entry *hash_table, int table_size, char *old_hash, char *new_hash,  int64_t page_addr);

int delete_entry(table_entry *hash_table, int table_size, char *hash);

void print_table(table_entry *hash_table, int size);
#endif
