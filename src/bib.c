/*
 * Copyright (c) 2024 Álan Crístoffer e Sousa.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "internal.h"

BIBEntry *bib_entry_create(void) {
  BIBEntry *entry = g_malloc(sizeof(BIBEntry));
  entry->key = NULL;
  entry->type = NULL;
  entry->properties = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  return entry;
}

BIBEntryList *bib_entry_list_create(void) {
  return g_ptr_array_new_with_free_func(bib_entry_free);
}

void bib_entry_free(gpointer ptr) {
  BIBEntry *entry = ptr;
  g_free(entry->key);
  g_free(entry->type);
  g_hash_table_destroy(entry->properties);
  g_free(entry);
}

void bib_entry_list_free(gpointer list) {
  g_ptr_array_set_free_func(list, bib_entry_free);
  g_ptr_array_free(list, TRUE);
}
