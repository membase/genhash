/*
 * Generic hash table implementation.
 *
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 * arch-tag: F52F1945-83FC-4FF4-9F40-38E5BBA662FB
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

enum UpdateType { MODIFICATION, NEW };

/**
 * Create a new generic hashtable.
 */
genhash_t* genHashInit(int, struct hash_ops);
/**
 * Free a gen hash.
 */
void genHashFree(genhash_t*);

/**
 * Store an item.
 */
void genHashStore(genhash_t*, const void*, const void*);
/**
 * Get an item.
 */
void* genHashFind(genhash_t*, const void*);
/**
 * Delete an item.
 */
int genHashDelete(genhash_t*, const void*);
/**
 * Delete all mappings of a given key.
 */
int genHashDeleteAll(genhash_t*, const void*);

/**
 * Update an item in-place.
 */
enum UpdateType genHashUpdate(genhash_t*, const void*, const void*);

/**
 * Update an item in-place.
 *
 * @param h hashtable
 * @param updateFunc function that will be called with the key and current
 *        value.  Should return the new value.
 * @param freeFunc function to free the return value returned by the update
 *        function
 * @param def default value
 */
enum UpdateType genHashFuncUpdate(genhash_t*, const void*,
	void *(*upd)(const void *, const void *), void (*fr)(void*),
	const void *def);

/**
 * Iterate all keys and values in a hash table.
 */
void genHashIter(genhash_t*,
	void (*iterfunc)(const void* key, const void* val, void *arg), void *arg);
/**
 * Iterate all values for a given key in a hash table.
 */
void genHashIterKey(genhash_t*, const void* key,
	void (*iterfunc)(const void* key, const void* val, void *arg), void *arg);

/**
 * Get the total number of entries in this hash table.
 */
int genHashSize(genhash_t*);

/**
 * Get the total number of entries in this hash table that map to the given
 * key.
 */
int genHashSizeForKey(genhash_t*, const void*);

/** Convenient hash function for strings */
int genHashStringHash(const void*);

#endif /* GENHASH_H */
