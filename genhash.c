/*
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 * arch-tag: 2433F082-79B4-427C-B897-2F847EDC2E3B
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "genhash.h"

/* Table of 32 primes by their distance from the nearest power of two */
static int prime_size_table[]={
	3, 7, 13, 23, 47, 97, 193, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157,
	98299, 196613, 393209, 786433, 1572869, 3145721, 6291449, 12582917,
	25165813, 50331653, 100663291, 201326611, 402653189, 805306357,
	1610612741
};

static int
estimateTableSize(int est)
{
	int rv=0;
	int magn=0;
	assert(est > 0);

	magn=(int)log((double)est)/log(2);
	magn--;
	if(magn < 0 ) {
		magn=0;
	}
	assert(magn < (sizeof(prime_size_table) / sizeof(int)));
	rv=prime_size_table[magn];
	return rv;
}

genhash_t* genHashInit(int est, struct hash_ops ops)
{
	genhash_t* rv=NULL;
	int size=0;

	assert(ops.hashfunc != NULL);
	assert(ops.hasheq != NULL);
	assert(ops.dupKey != NULL);
	assert(ops.dupValue != NULL);
	assert(ops.freeKey != NULL);
	assert(ops.freeValue != NULL);

	size=estimateTableSize(est);
	rv=calloc(1, sizeof(genhash_t)
		+ (size * sizeof(struct genhash_entry_t *)));
	assert(rv != NULL);
	rv->size=size;
	rv->ops=ops;

	return rv;
}

static void
freeBucket(genhash_t* h, struct genhash_entry_t* b)
{
	if(b != NULL) {
		freeBucket(h, b->next);
		h->ops.freeKey(b->key);
		h->ops.freeValue(b->value);
		free(b);
	}
}

void
genHashFree(genhash_t* h)
{
	if(h != NULL) {
		int i=0;
		for(i=0; i<h->size; i++) {
			freeBucket(h, h->buckets[i]);
		}
		free(h);
	}
}

void
genHashStore(genhash_t *h, const void* k, const void* v)
{
	int n=0;
	struct genhash_entry_t *p;

	assert(h != NULL);

	n=h->ops.hashfunc(k) % h->size;
	assert(n >= 0);
	assert(n < h->size);

	p=calloc(1, sizeof(struct genhash_entry_t));
	assert(p);

	p->key=h->ops.dupKey(k);
	p->value=h->ops.dupValue(v);

	p->next=h->buckets[n];
	h->buckets[n]=p;
}

static struct genhash_entry_t *
genHashFindEntry(genhash_t *h, const void* k)
{
	int n=0;
	struct genhash_entry_t *p;

	assert(h != NULL);
	n=h->ops.hashfunc(k) % h->size;
	assert(n >= 0);
	assert(n < h->size);

	p=h->buckets[n];
	for(p=h->buckets[n]; p && !h->ops.hasheq(k, p->key); p=p->next);
	return p;
}

void*
genHashFind(genhash_t *h, const void* k)
{
	struct genhash_entry_t *p;
	void *rv=NULL;

	p=genHashFindEntry(h, k);

	if(p) {
		rv=p->value;
	}
	return rv;
}

enum UpdateType
genHashUpdate(genhash_t* h, const void* k, const void* v)
{
	struct genhash_entry_t *p;
	enum UpdateType rv=0;

	p=genHashFindEntry(h, k);

	if(p) {
		h->ops.freeValue(p->value);
		p->value=h->ops.dupValue(v);
		rv=MODIFICATION;
	} else {
		genHashStore(h, k, v);
		rv=NEW;
	}

	return rv;
}

enum UpdateType
genHashFuncUpdate(genhash_t* h, const void* k,
	void *(*upd)(const void *, const void *),
	void (*fr)(void*), 
	const void *def)
{
	struct genhash_entry_t *p;
	enum UpdateType rv=0;

	p=genHashFindEntry(h, k);

	if(p) {
		void *newValue=upd(k, p->value);
		h->ops.freeValue(p->value);
		p->value=h->ops.dupValue(newValue);
		fr(newValue);
		rv=MODIFICATION;
	} else {
		void *newValue=upd(k, def);
		genHashStore(h, k, newValue);
		fr(newValue);
		rv=NEW;
	}

	return rv;
}

int
genHashDelete(genhash_t* h, const void* k)
{
	struct genhash_entry_t *deleteme=NULL;
	int n=0;
	int rv=0;

	assert(h != NULL);
	n=h->ops.hashfunc(k) % h->size;
	assert(n >= 0);
	assert(n < h->size);

	if(h->buckets[n] != NULL) {
		/* Special case the first one */
		if(h->ops.hasheq(h->buckets[n]->key, k)) {
			deleteme=h->buckets[n];
			h->buckets[n]=deleteme->next;
		} else {
			struct genhash_entry_t *p=NULL;
			for(p=h->buckets[n]; deleteme==NULL && p->next != NULL; p=p->next) {
				if(h->ops.hasheq(p->next->key, k)) {
					deleteme=p->next;
					p->next=deleteme->next;
				}
			}
		}
	}
	if(deleteme != NULL) {
		h->ops.freeKey(deleteme->key);
		h->ops.freeValue(deleteme->value);
		free(deleteme);
		rv++;
	}

	return rv;
}

int
genHashDeleteAll(genhash_t* h, const void* k)
{
	int rv=0;
	while(genHashDelete(h, k) == 1) {
		rv++;
	}
	return rv;
}

void
genHashIter(genhash_t* h,
	void (*iterfunc)(const void* key, const void* val, void *arg), void *arg)
{
	int i=0;
	struct genhash_entry_t *p=NULL;
	assert(h != NULL);

	for(i=0; i<h->size; i++) {
		for(p=h->buckets[i]; p!=NULL; p=p->next) {
			iterfunc(p->key, p->value, arg);
		}
	}
}

static void
countEntries(const void *key, const void *val, void *arg)
{
	int *count=(int *)arg;
	(*count)++;
}

int
genHashSize(genhash_t* h) {
	int rv=0;
	assert(h != NULL);
	genHashIter(h, countEntries, &rv);
	return rv;
}

int
genHashSizeForKey(genhash_t* h, const void* k)
{
	int rv=0;
	assert(h != NULL);
	genHashIterKey(h, k, countEntries, &rv);
	return rv;
}

void
genHashIterKey(genhash_t* h, const void* key,
	void (*iterfunc)(const void* key, const void* val, void *arg), void *arg)
{
	int n=0;
	struct genhash_entry_t *p=NULL;

	assert(h != NULL);
	n=h->ops.hashfunc(key) % h->size;
	assert(n >= 0);
	assert(n < h->size);

	for(p=h->buckets[n]; p!=NULL; p=p->next) {
		if(h->ops.hasheq(key, p->key)) {
			iterfunc(p->key, p->value, arg);
		}
	}
}

int
genHashStringHash(const void* p)
{
	int rv=5381;
	int i=0;
	char *str=(char *)p;

	for(i=0; str[i] != 0x00; i++) {
		rv = ((rv << 5) + rv) ^ str[i];
	}

	return rv;
}
