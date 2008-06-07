/*
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
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
str_eq(const void* p1, const void*p2)
{
	char *str1=(char *)p1;
	char *str2=(char *)p2;
	assert(str1 != NULL);
	assert(str2 != NULL);
	return strcmp(str1, str2) == 0;
}

void*
hash_str_dup(const void *p)
{
	char *str=(char *)p;
	assert(str != NULL);
	return strdup(str);
}

void
free_str(void *p)
{
	free(p);
}

static struct hash_ops
get_string_hash_ops()
{
	struct hash_ops rv;
	rv.hashfunc=genhash_string_hash;
	rv.hasheq=str_eq;
	rv.dupKey=hash_str_dup;
	rv.dupValue=hash_str_dup;
	rv.freeKey=free_str;
	rv.freeValue=free_str;

	return rv;
}

static void
test_construct()
{
	genhash_t* h=NULL;

	h=genhash_init(4, get_string_hash_ops());
	genhash_free(h);
}

static void
assert_hash_val(char *expected, genhash_t* h, const char* key)
{
	char *found=(char*)genhash_find(h, key);

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
get_test_hash()
{
	genhash_t* h=NULL;
	int i=0;

	h=genhash_init(4, get_string_hash_ops());
	for(i=0; i<26; i++) {
		genhash_store(h, kvs[i], kvs[i]);
		assert_hash_val(kvs[i], h, kvs[i]);
	}

	return h;
}

static void
test_simple()
{
	genhash_t* h=get_test_hash();
	int i=0;

	for(i=0; i<13; i++) {
		int deleted=genhash_delete(h, kvs[i*2]);
		assert(deleted == 1);
		deleted=genhash_delete(h, kvs[i*2]);
		assert(deleted == 0);
	}
	for(i=0; i<26; i++) {
		if(i % 2 == 0) {
			assert_hash_val(NULL, h, kvs[i]);
		} else {
			assert_hash_val(kvs[i], h, kvs[i]);
		}
	}
	genhash_free(h);
}

static void
test_update()
{
	genhash_t* h=get_test_hash();
	int type=0, i=0;

	for(i=0; i<26; i++) {
		assert_hash_val(kvs[i], h, kvs[i]);
		type=genhash_update(h, kvs[i], "updated");
		assert(type == MODIFICATION);
		assert_hash_val("updated", h, kvs[i]);
	}
	type=genhash_update(h, "newtest", "new");
	assert(type == NEW);
	assert_hash_val("new", h, "newtest");

	genhash_free(h);
}

static void
test_multiple_keys()
{
	genhash_t* h=genhash_init(4, get_string_hash_ops());
	int deleted=0;

	assert_hash_val(NULL, h, "x");
	genhash_store(h, "x", "a");
	genhash_store(h, "x", "b");

	assert_hash_val("b", h, "x");
	deleted=genhash_delete(h, "x");
	assert(deleted == 1);
	assert_hash_val("a", h, "x");
	deleted=genhash_delete(h, "x");
	assert(deleted == 1);
	assert_hash_val(NULL, h, "x");
	deleted=genhash_delete(h, "x");
	assert(deleted == 0);

	genhash_store(h, "x", "a");
	genhash_store(h, "x", "b");
	genhash_store(h, "y", "yz");

	assert(genhash_size(h) == 3);
	assert(genhash_size_for_key(h, "x") == 2);

	deleted=genhash_delete_all(h, "x");
	assert(deleted == 2);

	genhash_free(h);
}

static void*
update_fun(const void* k, const void*v)
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
test_function_update()
{
	genhash_t* h=genhash_init(4, get_string_hash_ops());
	int type=0;

	assert_hash_val(NULL, h, "x");
	type=genhash_fun_update(h, "x", update_fun, free_str, NULL);
	assert(type == NEW);
	assert_hash_val("", h, "x");
	type=genhash_fun_update(h, "x", update_fun, free_str, NULL);
	assert(type == MODIFICATION);
	assert_hash_val("x", h, "x");
	type=genhash_fun_update(h, "x", update_fun, free_str, NULL);
	assert(type == MODIFICATION);
	assert_hash_val("xx", h, "x");

	assert(genhash_size(h) == 1);

	genhash_free(h);
}

int main(int argc, char **argv)
{
	test_construct();
	test_simple();
	test_update();
	test_multiple_keys();
	test_function_update();
	return 0;
}
