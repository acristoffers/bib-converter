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

#pragma once

#include <stdbool.h>

#include <gio/gio.h>
#include <glib.h>

////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///                                  Types                                   ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

struct options {
  gboolean bibtex;
  gboolean version;
  gchar **rest;
};

struct BIBEntry {
  gchar *type;
  gchar *key;
  GHashTable *properties;
};

typedef GPtrArray BIBEntryList;
typedef struct BIBEntry BIBEntry;

////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///                                Functions                                 ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

struct options parse_options(int argc, char **argv);

BIBEntryList *bib_parse(const GString *bibfile, GError **error);

BIBEntry *bib_entry_create(void);
BIBEntryList *bib_entry_list_create(void);
void bib_entry_free(gpointer entry);
void bib_entry_list_free(gpointer list);

GString *bib_entry_print(BIBEntry *entry, gboolean bibtex);
GString *bib_entry_list_print(BIBEntryList *list, gboolean bibtex);

void free_regex(void);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(BIBEntry, bib_entry_free)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(BIBEntryList, bib_entry_list_free)
