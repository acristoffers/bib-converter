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
