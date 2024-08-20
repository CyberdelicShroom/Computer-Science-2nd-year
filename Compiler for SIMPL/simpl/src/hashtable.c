/**
 * @file    hashtable.c
 * @brief   A generic hash table.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2021-08-23
 */

#include "hashtable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define INITIAL_DELTA_INDEX 4
#define PRINT_BUFFER_SIZE 1024

/** an entry in the hash table */
typedef struct htentry HTentry;
struct htentry {
	void *key;		   /*<< the key                      */
	void *value;	   /*<< the value                    */
	HTentry *next_ptr; /*<< the next entry in the bucket */
};

/** a hash table container */
struct hashtab {
	/** a pointer to the underlying table                              */
	HTentry **table;
	/** the current size of the underlying table                       */
	unsigned int size;
	/** the current number of entries                                  */
	unsigned int num_entries;
	/** the maximum load factor before the underlying table is resized */
	float max_loadfactor;
	/** the index into the delta array                                 */
	unsigned short idx;
	/** a pointer to the hash function                                 */
	unsigned int (*hash)(void *, unsigned int);
	/** a pointer to the comparison function                           */
	int (*cmp)(void *, void *);
};

/* --- function prototypes -------------------------------------------------- */

/* TODO: For the following functions, refer to the TODO note at the end of the
 * file.
 */
static int getsize(HashTab *ht);
static HTentry **talloc(int tsize);
static void rehash(HashTab *ht);

/* TODO: For this implementation, we want to ensure we *always* have a hash
 * table that is of prime size.  To that end, the next array stores the
 * different between a power of two and the largest prime less than that
 * particular power-of-two.  When you rehash, compute the new prime size using
 * the following array.
 */

/** the array of differences between a power-of-two and the largest prime less
 * than that power-of-two.                                                */
unsigned short delta[] = {0,  0, 1, 1, 3, 1, 3, 1,	5, 3,  3, 9,  3,  1, 3,	 19,
						  15, 1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1};

#define MAX_IDX (sizeof(delta) / sizeof(short))

/* --- hash table interface ------------------------------------------------- */

HashTab *ht_init(float loadfactor, unsigned int (*hash)(void *, unsigned int),
				 int (*cmp)(void *, void *))
{
	HashTab *ht;
	/*unsigned int i;*/
	ht = (HashTab *)malloc(sizeof(HashTab));
	ht->idx = INITIAL_DELTA_INDEX;
	ht->size = (1 << ht->idx) - delta[ht->idx];
	ht->table = (HTentry **)malloc(sizeof(HTentry) * ht->size);
	if (ht == NULL || ht->table == NULL) {
		free(ht->table);
		free(ht);
		return NULL;
	}
	ht->num_entries = 0;
	ht->max_loadfactor = loadfactor;
	ht->hash = hash;
	ht->cmp = cmp;

	/* TODO:
	 * - Initialise a hash table structure by calling an allocation function
	 *   twice:
	 *   (1) once to allocate space for a HashTab variable, and
	 *   (2) once to allocate space for the table field of this new HashTab
	 *       variable.
	 * - If any allocation fails, free anything that has already been allocated
	 *   successfully, and return NULL.
	 * - Also set up the other fields of the newly-allocated HashTab structure
	 *   appropriately.
	 */

	return ht;
}

int ht_insert(HashTab *ht, void *key, void *value)
{
	int k;
	HTentry *p, *h;
	p = (HTentry *)malloc(sizeof(HTentry));
	if (p == NULL) {
		return HASH_TABLE_NO_SPACE_FOR_NODE;
	}
	k = ht->hash(key, ht->size);
	p->key = key;
	p->value = value;
	p->next_ptr = NULL;
	ht->num_entries++;

	if (ht->table[k] == NULL) {
		ht->table[k] = p;
	} else {
		h = ht->table[k];
		while (h) {
			if (ht->cmp(h->key, p->key) == 0) {
				return HASH_TABLE_KEY_VALUE_PAIR_EXISTS;
			} else if (h->next_ptr == NULL) {
				h->next_ptr = p;
				break;
			} else {
				h = h->next_ptr;
			}
		}
	}

	float loadfactor = (float)ht->num_entries / (float)ht->size;
	if (loadfactor > ht->max_loadfactor) {
		/*printf("REHASH CALLED: ");*/
		rehash(ht);
	}
	return EXIT_SUCCESS;
	/* TODO: Insert a new key--value pair, rehashing if necessary.  The best way
	 * to go about rehashing is to put the necessary elements into a static
	 * function called rehash.  Remember to free space (the "old" table) you not
	 * use any longer.  Also, if something goes wrong, use the #define'd
	 * constants in hashtable.h for return values; remember, unless it runs out
	 * of memory, no operation on a hash table may terminate the program.
	 */
}

Boolean ht_search(HashTab *ht, void *key, void **value)
{
	int k;
	HTentry *p;

	/* TODO: Nothing!  This function is complete, and should explain by example
	 * how the hash table looks and must be accessed.
	 */

	k = ht->hash(key, ht->size);
	for (p = ht->table[k]; p; p = p->next_ptr) {
		if (ht->cmp(key, p->key) == 0) {
			*value = p->value;
			break;
		}
	}

	return (p ? TRUE : FALSE);
}

Boolean ht_free(HashTab *ht, void (*freekey)(void *k), void (*freeval)(void *v))
{
	unsigned int i;
	HTentry *p /*, *q*/;

	/* free the nodes in the buckets */
	/* TODO */
	for (i = 0; i < ht->size; i++) {
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			freekey(p->key);
			freeval(p->value);
		}
	}
	/* free the table and container */
	/* TODO */
	free(ht->table);
	free(ht);

	return EXIT_SUCCESS;
}

void ht_print(HashTab *ht, void (*keyval2str)(void *k, void *v, char *b))
{
	unsigned int i;
	HTentry *p;
	char buffer[PRINT_BUFFER_SIZE];

	/* TODO: This function is complete and useful for testing, but you have to
	 * write your own keyval2str function if you want to use it.
	 */

	for (i = 0; i < ht->size; i++) {
		printf("bucket[%2i]", i);
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			keyval2str(p->key, p->value, buffer);
			printf(" --> %s", buffer);
		}
		printf(" --> NULL\n");
	}
}

/* --- utility functions ---------------------------------------------------- */

/* TODO: I suggest completing the following helper functions for use in the
 * global functions ("exported" as part of this unit's public API) given above.
 * You may, however, elect not to use them, and then go about it in your own way
 * entirely.  The ball is in your court, so to speak, but remember: I have
 * suggested using these functions for a reason -- they should make your life
 * easier.
 */

static int getsize(HashTab *ht)
{
	int i, new_size;
	i = ht->idx + 1;
	new_size = (1 << i) - delta[i];
	return new_size;
	/* TODO: Compute the next prime size of the hash table. */
}

static HTentry **talloc(int tsize)
{
	/* TODO: Allocate space for one hash table entry. */
	return emalloc(tsize);
}

static void rehash(HashTab *ht)
{
	HashTab *rh;
	HTentry *p;
	p = malloc(sizeof(HTentry));
	int i;
	int old_size = ht->size;
	rh = malloc(sizeof(HashTab));
	int new_size = getsize(ht);
	ht->idx = ht->idx + 1;
	ht->size = new_size;
	rh->table = talloc(sizeof(HTentry) * new_size);
	rh->num_entries = 0;
	rh->max_loadfactor = ht->max_loadfactor;
	rh->idx = ht->idx;
	rh->hash = ht->hash;
	rh->cmp = ht->cmp;
	rh->size = new_size;

	for (i = 0; i < old_size; i++) {
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			int ret;
			if ((ret = ht_insert(rh, p->key, p->value)) == EXIT_SUCCESS) {
			}
		}
	}

	ht->table = talloc(sizeof(HTentry) * ht->size);

	for (i = 0; i < new_size; i++) {
		ht->table[i] = rh->table[i];
	}

	free(rh->table);
	free(rh);
	/*printf("idx = %d\n", ht->idx);
	printf("max_lf = %f\n", ht->max_loadfactor);
	printf("num_entries = %d\n", ht->num_entries);
	printf("size = %d\n", ht->size);*/

	/* TODO: Rehash the hash table by
	 * (1) allocating a new table that uses as size the next prime in the
	 *     "almost-double" array,
	 * (2) moving the entries in the existing table to appropriate positions in
	 *     the new table, and
	 * (3) freeing the old table.
	 */
}
