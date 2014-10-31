#include "migration/hashing.h"

void hash(unsigned char * buffer, int len, unsigned char * sha256sum){
    sha256_context ctx;
    sha256_starts(&ctx);
    sha256_update(&ctx, buffer, len);
    sha256_finish(&ctx, sha256sum);
}

unsigned int getindex(unsigned char * sha256sum, int table_size){

    int j;
    for( j = 0; j < 32; j++ )
    {
        printf( "%02x", sha256sum[j] );
    }
    printf("\n");

    /*sha256sum[28] = 0;
    sha256sum[29] = 0;
    sha256sum[30] = 0;
    sha256sum[31] = 23;
    */
    
    unsigned int lastint;
    lastint = (sha256sum[28] << 24) + (sha256sum[29] << 16) + (sha256sum[30] << 8) + (sha256sum[31]); 

    unsigned masked_index;
    masked_index = lastint % table_size;

    return masked_index;
}

void print_sha256(unsigned char * sha256sum){
    int j;
    for( j = 0; j < 32; j++ )
    {
        printf( "%02x", sha256sum[j] );
    }
    printf("\n");
}

/*struct table_entry {
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
*/
table_entry* create_table(uint32_t size) {
  table_entry* hash_table = (table_entry*) calloc(size, sizeof(table_entry));
  int i;
  for(i=0; i < size; i++) {
    hash_table[i].page_num = -1;
    hash_table[i].is_empty = 1;
  }
  return hash_table;
}

void insert_entry(table_entry *hash_table, uint32_t table_size, uint32_t index, char *hash, uint32_t page_num) {
  if(index >= table_size) {
    printf("Insertion error : index out of bounds!\n");
  }
  else if(hash_table[index].is_empty) {
    memcpy(hash_table[index].hash_val, hash, HASHSIZE);
    hash_table[index].page_num = page_num;
    hash_table[index].is_empty = 0;
  }

  //Slot not empty. Probe linearly.
  else {
    uint32_t i = index+1;
    while(i % table_size != index) {
      if(hash_table[i].is_empty) {
        memcpy(hash_table[i].hash_val, hash, HASHSIZE);
        hash_table[i].page_num = page_num;
        hash_table[i].is_empty = 0;
        break;
      }
      else i++;
    }
    if(i == index) {
      printf("Table insertion error : No empty slots!\n");
    }
  }
}

uint32_t find_entry(table_entry *hash_table, char *hash, uint32_t table_size) {
 uint32_t i;
 for(i = 0; i < table_size; i++) {
  if(memcmp(hash_table[i].hash_val, hash, HASHSIZE) == 0) {
    return i;
  }
 }
 return -1;
}

void update_entry(table_entry *hash_table, uint32_t table_size, char *old_hash, char *new_hash,  uint32_t page_num) {
  uint32_t i = find_entry(hash_table, old_hash, table_size);
  if(i != -1) {
    memcpy(hash_table[i].hash_val, new_hash, HASHSIZE);
    hash_table[i].page_num = page_num;
    hash_table[i].is_empty = 0;
  }
  else {
    printf("Table entry error : Entry not found!\n");
  }
}

void delete_entry(table_entry *hash_table, uint32_t table_size, char *hash) {
  uint32_t i = find_entry(hash_table, hash, table_size);
  if(i != -1) {
    hash_table[i].page_num = -1;
    hash_table[i].is_empty = 1;
  }
  else {
    printf("Table entry error : Entry not found!\n");
  }
}

void print_table(table_entry *hash_table, uint32_t size) {
  int i;
  for(i = 0; i < size; i++) {
    printf("Index : %d, Hash :", i);
    print_sha256(hash_table[i].hash_val);
    printf(", Occupancy : %d, Page No : %d\n",hash_table[i].is_empty, hash_table[i].page_num);
  }
}
/*
int main_check() {
    char buf[100] = "abcdefght 323";
    int len = 100;

    unsigned char sha256sum[32];
    hash(buf, len, sha256sum);
    print_sha256(sha256sum);

    //int hashmask = HASH_INDEX_MASK;
    printf("hashmask %d\n", hashmask);
    printf("hashindex %u\n", getindex(sha256sum));
    unsigned int index =  getindex(sha256sum);
    printf("hashindex %u\n", index);

  table_entry *ht = create_table(TABLE_SIZE);
  insert_entry(ht, TABLE_SIZE, index, sha256sum, 1);
  print_table(ht, TABLE_SIZE);


    char buf2[100] = "bcdefght 323";
    int len2 = 100;

    unsigned char sha256sum2[32];
    hash(buf2, len2, sha256sum2);
    print_sha256(sha256sum2);
  //update_entry(ht, TABLE_SIZE, sha256sum2, sha256sum, 2);
  insert_entry(ht, TABLE_SIZE, index, sha256sum2, 3);
  print_table(ht, TABLE_SIZE);

  delete_entry(ht, TABLE_SIZE, sha256sum);
  insert_entry(ht, TABLE_SIZE, index, sha256sum, 6);
  print_table(ht, TABLE_SIZE);
  return 0;
}
*/
