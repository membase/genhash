/*
 * Generic hash table implementation.
 *
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 */

#ifndef GENHASH_H
#define GENHASH_H 1

struct genhash_entry_t {
    void *key;
    void *value;
    struct genhash_entry_t *next;
};

struct __genhash_struct;
/**
 * The main hash table type.
 */
typedef struct __genhash_struct genhash_t;

/**
 * Operations on keys and values in the hash table.
 */
struct hash_ops {
    /**
     * Function to compute a hash for the given value.
     */
    int   (*hashfunc)(const void *);
    /**
     * Function that returns true if the given keys are equal.
     */
    int   (*hasheq)(const void *, const void *);
    /**
     * Function to duplicate a key for storage.
     */
    void* (*dupKey)(const void *);
    /**
     * Function to duplicate a value for storage.
     */
    void* (*dupValue)(const void *);
    /**
     * Function to free a key.
     */
    void  (*freeKey)(void *);
    /**
     * Function to free a value.
     */
    void  (*freeValue)(void *);
};

struct __genhash_struct {
    size_t size;
    struct hash_ops ops;
    struct genhash_entry_t *buckets[0];
};

enum update_type { MODIFICATION, NEW };

/**
 * Create a new generic hashtable.
 */
genhash_t* genhash_init(int, struct hash_ops);
/**
 * Free a gen hash.
 */
void genhash_free(genhash_t*);

/**
 * Store an item.
 */
void genhash_store(genhash_t*, const void*, const void*);
/**
 * Get an item.
 */
void* genhash_find(genhash_t*, const void*);
/**
 * Delete an item.
 */
int genhash_delete(genhash_t*, const void*);
/**
 * Delete all mappings of a given key.
 */
int genhash_delete_all(genhash_t*, const void*);

/**
 * Update an item in-place.
 */
enum update_type genhash_update(genhash_t*, const void*, const void*);

/**
 * Update an item in-place.
 *
 * @param h hashtable
 * @param update_fun function that will be called with the key and current
 *        value.  Should return the new value.
 * @param free_fun function to free the return value returned by the update
 *        function
 * @param def default value
 */
enum update_type genhash_fun_update(genhash_t*, const void*,
                                    void *(*upd)(const void *, const void *),
                                    void (*fr)(void*),
                                    const void *def);

/**
 * Iterate all keys and values in a hash table.
 */
void genhash_iter(genhash_t*,
                  void (*iterfunc)(const void* key, const void* val, void *arg), void *arg);
/**
 * Iterate all values for a given key in a hash table.
 */
void genhash_iter_key(genhash_t*, const void* key,
                      void (*iterfunc)(const void* key, const void* val, void *arg), void *arg);

/**
 * Get the total number of entries in this hash table.
 */
int genhash_size(genhash_t*);

/**
 * Get the total number of entries in this hash table that map to the given
 * key.
 */
int genhash_size_for_key(genhash_t*, const void*);

/** Convenient hash function for strings */
int genhash_string_hash(const void*);

#endif /* GENHASH_H */
