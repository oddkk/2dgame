#include "idlookuptable.h"
#include "murmurhash.h"
#include "utils.h"
#include <stdlib.h>

#define MURMURHASH_SEED 1

// TODO: Implement rehash function.

int id_lookup_table_insert(struct id_lookup_table *table, struct string name,
			   unsigned int id)
{
	uint32_t hash;
	struct name_id_pair **current;
	struct name_id_pair *new_entry;

	if (!table->buckets || table->num_buckets == 0) {
		table->num_buckets = 32;
		table->buckets = calloc(table->num_buckets,
								sizeof(struct name_id_pair **));
	}

	hash = murmurhash((char *)name.data, name.length, MURMURHASH_SEED);

	current = &table->buckets[hash % table->num_buckets];

	// Search the table to figure out if the name already exist. If
	// not, current will point to where the new entry should be
	// inserted.
	while (*current && (*current)->hash <= hash) {
		if ((*current)->hash == hash
		    && string_equals(name, (*current)->name)) {
			return -1;
		}
		current = &(*current)->next_in_bucket;
	}

	if (!table->free_list) {
		struct id_lookup_table_page *new_page;

		new_page = calloc(1, sizeof(struct id_lookup_table_page));

		if (!new_page) {
			print_error("id lookup table",
						"Could not allocate memory for new page.");
			return -1;
		}

		new_page->previous = table->last_page;
		table->last_page = new_page;

		// Set up the free list
		for (size_t i = 0; i < ID_LOOKUP_TABLE_PAGE_CAPACITY - 1; i++) {
			new_page->entries[i].next_in_bucket = &new_page->entries[i + 1];
		}
		new_page->entries[ID_LOOKUP_TABLE_PAGE_CAPACITY - 1].next_in_bucket = table->free_list;
		table->free_list = &new_page->entries[0];
	}

	new_entry = table->free_list;
	table->free_list = new_entry->next_in_bucket;

	new_entry->hash = hash;
	new_entry->id = id;

	new_entry->name = duplicate_string(name);

	new_entry->next_in_bucket = *current;
	*current = new_entry;

	table->count += 1;

	return 0;
}

int id_lookup_table_lookup(struct id_lookup_table *table, struct string name)
{
	uint32_t hash;
	struct name_id_pair **current;

	if (!table->buckets || table->num_buckets == 0) {
		return -1;
	}

	hash = murmurhash((char *)name.data, name.length, MURMURHASH_SEED);

	current = &table->buckets[hash % table->num_buckets];

	while (*current && (*current)->hash <= hash) {
		if ((*current)->hash == hash
		    && string_equals(name, (*current)->name)) {
			return (*current)->id;
		}
		current = &(*current)->next_in_bucket;
	}

	// The name was not found.
	return -1;
}

int id_lookup_table_delete(struct id_lookup_table *table, struct string name) {
	uint32_t hash;
	struct name_id_pair **current;

	if (!table->buckets || table->num_buckets == 0) {
		return -1;
	}

	hash = murmurhash((char *)name.data, name.length, MURMURHASH_SEED);

	current = &table->buckets[hash % table->num_buckets];

	while (*current && (*current)->hash <= hash) {
		if ((*current)->hash == hash
		    && string_equals(name, (*current)->name)) {
			struct name_id_pair *next_current;

			next_current = (*current)->next_in_bucket;

			(*current)->next_in_bucket = table->free_list;
			table->free_list = (*current);

			*current = next_current;

			table->count -= 1;

			return 0;
		}
		current = &(*current)->next_in_bucket;
	}

	// The name was not found.
	return -1;
}
