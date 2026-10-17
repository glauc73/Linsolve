#define NUMBER_OF_LETTERS 'z' - 'a' + 1
#define TABLE_SIZE 26*11*11
#define SIZE_KEY 3

typedef struct{
    double coef;
    size_t col;
}pair;

typedef struct{
    char key[SIZE_KEY + 1];
    pair term;
}slots;

typedef struct{
    slots *buckets;
}hash_table;

static inline int hash(char* key){
    if(key[0] < 'a' || key[0] > 'z') return -1;
    int h = 11*11*(key[0] - 'a');

    if(key[1] >= '0' && key[1] <= '9')
        h += 11*(key[1] - '0' + 1);

    if(key[2] >= '0' && key[2] <= '9')
        h += key[2] - '0' + 1;

    return h;
}

static inline hash_table new_hash_table(){
    hash_table table;
    table.buckets = malloc(TABLE_SIZE * sizeof(slots));
    if(!table.buckets){
        printf("memoria insuficiente!\n");
        exit(EXIT_FAILURE);
    }
    return table;
}

static inline void delete_hash_table(hash_table* ht){
    if(!ht) return;
    free(ht->buckets);
    ht->buckets = NULL;
}

static inline void get_key(hash_table* ht, int index, char* key){
    if(index > TABLE_SIZE || !key) return;
    strncpy(key, ht->buckets[index].key, SIZE_KEY);
}


static inline int hash_insert(hash_table *ht,  char* key, pair term){
    int index = hash(key);
    if(index < 0) return 0;
    ht->buckets[index].term.coef = term.coef;
    ht->buckets[index].term.col = term.col;
    strncpy(ht->buckets[index].key, key, SIZE_KEY);
    ht->buckets[index].key[SIZE_KEY] = '\0';
    return 1;
}

static inline int hash_update(hash_table *ht,  char* key, pair term){
    int index = hash(key);
    if(index < 0) return 0;

    if(strncmp(ht->buckets[index].key, key, SIZE_KEY) != 0){
        hash_insert(ht, key, term);
        return 0;
    }
    ht->buckets[index].term = term;
    return 1;
}

static inline int hash_contains(hash_table* ht, char* key){
    int index = hash(key);
    return (strncmp(ht->buckets[index].key, key, SIZE_KEY) == 0);
}

static inline pair hash_get(hash_table* ht, char* key){
    int index = hash(key);
    return ht->buckets[index].term;
}

static inline double hash_get_coef(hash_table* ht, char* key){
    int index = hash(key);
    return ht->buckets[index].term.coef;
}

static inline size_t hash_get_col(hash_table* ht, char* key){
    int index = hash(key);
    return ht->buckets[index].term.col;
}

static inline pair hash_at(hash_table* ht, int index){
    return  ht->buckets[index].term;
}
