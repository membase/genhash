/*
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>

 * arch-tag: D3539587-3E66-4DB3-9DB6-DDF5EF5651B9
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "genhash.h"

static char *kvs[]={
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
};

static int
strEq(const void* p1, const void*p2)
{
	char *str1=(char *)p1;
	char *str2=(char *)p2;
	assert(str1 != NULL);
	assert(str2 != NULL);
	return strcmp(str1, str2) == 0;
}

void*
hashStrDup(const void *p)
{
	char *str=(char *)p;
	assert(str != NULL);
	return strdup(str);
}

void
freeStr(void *p)
{
	free(p);
}

static struct hash_ops
getStringHashOps()
{
	struct hash_ops rv;
	rv.hashfunc=genHashStringHash;
	rv.hasheq=strEq;
	rv.dupKey=hashStrDup;
	rv.dupValue=hashStrDup;
	rv.freeKey=freeStr;
	rv.freeValue=freeStr;

	return rv;
}

static void
testConstruct()
{
	genhash_t* h=NULL;

	h=genHashInit(4, getStringHashOps());
	genHashFree(h);
}

static void
assertHashVal(char *expected, genhash_t* h, const char* key)
{
	char *found=(char*)genHashFind(h, key);

	if(expected == NULL) {
		if(found != NULL) {
			fprintf(stderr, "Expected NULL for ``%s'', got ``%s''\n",
				key, found);
			abort();
		}
	} else {
		if(strcmp(expected, found) != 0) {
			fprintf(stderr, "Expected ``%s'' for ``%s'', got ``%s''\n",
				expected, key, found);
			abort();
		}
	}
}

static genhash_t*
getTestHash()
{
	genhash_t* h=NULL;
	int i=0;

	h=genHashInit(4, getStringHashOps());
	for(i=0; i<26; i++) {
		genHashStore(h, kvs[i], kvs[i]);
		assertHashVal(kvs[i], h, kvs[i]);
	}

	return h;
}

static void
testSimple()
{
	genhash_t* h=getTestHash();
	int i=0;

	for(i=0; i<13; i++) {
		int deleted=genHashDelete(h, kvs[i*2]);
		assert(deleted == 1);
		deleted=genHashDelete(h, kvs[i*2]);
		assert(deleted == 0);
	}
	for(i=0; i<26; i++) {
		if(i % 2 == 0) {
			assertHashVal(NULL, h, kvs[i]);
		} else {
			assertHashVal(kvs[i], h, kvs[i]);
		}
	}
	genHashFree(h);
}

static void
testUpdate()
{
	genhash_t* h=getTestHash();
	int type=0, i=0;

	for(i=0; i<26; i++) {
		assertHashVal(kvs[i], h, kvs[i]);
		type=genHashUpdate(h, kvs[i], "updated");
		assert(type == MODIFICATION);
		assertHashVal("updated", h, kvs[i]);
	}
	type=genHashUpdate(h, "newtest", "new");
	assert(type == NEW);
	assertHashVal("new", h, "newtest");

	genHashFree(h);
}

static void
testMultipleKeys()
{
	genhash_t* h=genHashInit(4, getStringHashOps());
	int deleted=0;

	assertHashVal(NULL, h, "x");
	genHashStore(h, "x", "a");
	genHashStore(h, "x", "b");

	assertHashVal("b", h, "x");
	deleted=genHashDelete(h, "x");
	assert(deleted == 1);
	assertHashVal("a", h, "x");
	deleted=genHashDelete(h, "x");
	assert(deleted == 1);
	assertHashVal(NULL, h, "x");
	deleted=genHashDelete(h, "x");
	assert(deleted == 0);

	genHashStore(h, "x", "a");
	genHashStore(h, "x", "b");
	genHashStore(h, "y", "yz");

	assert(genHashSize(h) == 3);
	assert(genHashSizeForKey(h, "x") == 2);

	deleted=genHashDeleteAll(h, "x");
	assert(deleted == 2);

	genHashFree(h);
}

static void*
updateFunction(const void* k, const void*v)
{
	char *rv=NULL;

	assert(strcmp((char*)k, "x") == 0);

	if(v == NULL) {
		rv=strdup("");
	} else {
		rv=calloc(strlen((char*)v)+2, sizeof(char));
		assert(rv);
		strcpy(rv, (char*)v);
		strcat(rv, "x");
	}

	return rv;
}

static void
testFunctionUpdate()
{
	genhash_t* h=genHashInit(4, getStringHashOps());
	int type=0;

	assertHashVal(NULL, h, "x");
	type=genHashFuncUpdate(h, "x", updateFunction, freeStr, NULL);
	assert(type == NEW);
	assertHashVal("", h, "x");
	type=genHashFuncUpdate(h, "x", updateFunction, freeStr, NULL);
	assert(type == MODIFICATION);
	assertHashVal("x", h, "x");
	type=genHashFuncUpdate(h, "x", updateFunction, freeStr, NULL);
	assert(type == MODIFICATION);
	assertHashVal("xx", h, "x");

	assert(genHashSize(h) == 1);

	genHashFree(h);
}

int main(int argc, char **argv)
{
	testConstruct();
	testSimple();
	testUpdate();
	testMultipleKeys();
	testFunctionUpdate();
	return 0;
}
