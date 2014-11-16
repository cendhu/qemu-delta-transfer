#include "migration/hashing.h"

void hash(unsigned char * buffer, int len, unsigned char * sha256sum){
    sha256_context ctx;
    sha256_starts(&ctx);
    sha256_update(&ctx, buffer, len);
    sha256_finish(&ctx, sha256sum);
    /*int i;
    for(i=0; i<32; i++) {
      sha256sum[i] = i;
    }*/
}

unsigned int getindex(unsigned char * sha256sum, int table_size){

    int j;
    /*for( j = 0; j < 32; j++ )
    {
        printf( "%02x", sha256sum[j] );
    }
    printf("\n");
*/
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
  int64_t page_addr;
  bool is_empty;
};

typedef struct table_entry table_entry;

struct hash_table_t {
  table_entry *table;
  int64_t size;
};

typedef struct hash_table_t hash_table_t;
*/
table_entry* create_table(int size) {
  table_entry* hash_table = (table_entry*) calloc(size, sizeof(table_entry));
  int i;
  for(i=0; i < size; i++) {
    hash_table[i].page_addr = -1;
    hash_table[i].is_empty = 1;
  }
  return hash_table;
}

table_entry_node* create_table_chained(int size) {
  table_entry_node *ht_chained = (table_entry_node*) calloc(size, sizeof(table_entry_node));
  int i;
  for(i=0; i < size; i++) {
    ht_chained[i].addr = -1;
    ht_chained[i].is_empty = 1;
    ht_chained[i].next = NULL;
  }
  return ht_chained;
}

void insert_entry_c(table_entry_node *hash_table, int table_size, char *hash, int64_t page_addr) {
  int index = getindex(hash, table_size);
  if(index >= table_size) {
    printf("Insertion error : index out of bounds!\n");
  }
  else {
    table_entry_node *n = &hash_table[index]; //Find the first empty slot or the last slot in the current list.
    while( ! (n->is_empty || n->next == NULL)) {
      n = n->next;
    }
    if(n->is_empty) { //If this slot is empty, insert here.
      n->addr = page_addr;
      n->is_empty = 0;
      memcpy(n->hash_val, hash, HASHSIZE);
    } 
    else {  //n->next is null //Otherwise create a new entry and make it the next entry of the current last entry. 
      table_entry_node *n1 = (table_entry_node*) malloc(sizeof(table_entry_node));
      n1->is_empty = 0;
      n1->addr = page_addr;
      memcpy(n1->hash_val, hash, HASHSIZE);
      n1->next = NULL;
      n->next = n1;
    }
  }
}

int delete_entry_c(table_entry_node *hash_table, int table_size, char *hash, int64_t page_addr) {
  int index = getindex(hash, table_size);
  
  table_entry_node *n = &hash_table[index];
  while(n && !(memcmp(n->hash_val, hash, HASHSIZE) == 0 && page_addr == n->addr)) {   //Find the slot that matches.
    n = n->next;
  }
  if(n == NULL) {  //Slot not found till the end.
    printf("Entry not found!\n");
  }  
  else {  //Slot found, delete.
    n->is_empty = 1;
  }
}

table_entry_node *get_list_c(table_entry_node *hash_table, char *hash, int table_size) {
  int index = getindex(hash, table_size);
  return &hash_table[index];
}

table_entry_node *find_next_c(table_entry_node *n, char *hash, int64_t page_addr) {
  if(n == NULL) return NULL;

  while(! (n->is_empty == 0 && 
           memcmp(n->hash_val, hash, HASHSIZE) == 0 && 
           (page_addr & PAGE_OFFSET_MASK) == (n->addr & PAGE_OFFSET_MASK))) {   //Find the slot that matches.
    n = n->next;
    if(n == NULL) break;
  }

  return n; //NULL means reached end and no further match found
}

//=============================================================================

void insert_entry(table_entry *hash_table, int table_size, char *hash, int64_t page_addr) {
  int index = getindex(hash, table_size);
  if(index >= table_size) {
    printf("Insertion error : index out of bounds!\n");
  }
  else if(hash_table[index].is_empty) {
    memcpy(hash_table[index].hash_val, hash, HASHSIZE);
    hash_table[index].page_addr = page_addr;
    hash_table[index].is_empty = 0;
  }

  //Slot not empty. Probe linearly.
  else {
    int i = index+1;
    while(i % table_size != index) {
      if(hash_table[i % table_size].is_empty) {
        memcpy(hash_table[i % table_size].hash_val, hash, HASHSIZE);
        hash_table[i % table_size].page_addr = page_addr;
        hash_table[i % table_size].is_empty = 0;
        break;
      }
      else i++;
    }
    if(i % table_size == index) {
      printf("Table insertion error : No empty slots!\n");
    }
  }
}

int find_entry(table_entry *hash_table, char *hash, int table_size) {
 int i,j=0;
 i = getindex(hash, table_size);
 for(j = i; j < table_size; j++) {
  if(!hash_table[j].is_empty && memcmp(hash_table[j].hash_val, hash, HASHSIZE) == 0) {
    return j;
  }
 }
 for(j = 0; j < i; j++) {
  if(!hash_table[j].is_empty && memcmp(hash_table[j].hash_val, hash, HASHSIZE) == 0) {
    return j;
  }
 }
 return -1;
}

void update_entry(table_entry *hash_table, int table_size, char *old_hash, char *new_hash,  int64_t page_addr) {
  int i = find_entry(hash_table, old_hash, table_size);
  if(i != -1) {
    memcpy(hash_table[i].hash_val, new_hash, HASHSIZE);
    hash_table[i].page_addr = page_addr;
    hash_table[i].is_empty = 0;
  }
  else {
    printf("Table entry error : Entry not found!\n");
  }
}

int delete_entry(table_entry *hash_table, int table_size, char *hash) {
  int i = find_entry(hash_table, hash, table_size);
  if(i != -1) {
    hash_table[i].page_addr = -1;
    hash_table[i].is_empty = 1;
    return 1;
  }
  else {
    //printf("Table entry error : Entry not found!\n");
    return -1;
  }
}

void print_table(table_entry *hash_table, int size) {
  int i;
  for(i = 0; i < size; i++) {
    printf("Index : %d, Hash :", i);
    print_sha256(hash_table[i].hash_val);
    printf(", Occupancy : %d, Page No : %d\n",hash_table[i].is_empty, hash_table[i].page_addr);
  }
}
/*
int main() {
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
    table_entry_node *ht = create_table_chained(TABLE_SIZE);
    insert_entry_c(ht, TABLE_SIZE, index, sha256sum, 1);
   /*   
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
}*/

