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

#include <stdio.h>

static GRegex *date_regex = NULL;

GString *bib_property_print(gchar *key, gchar *val, gsize length, gboolean bibtex) {
  GString *property = g_string_new("");

  if (bibtex && g_ascii_strcasecmp(key, "date") == 0) {
    if (date_regex == NULL) {
      date_regex = g_regex_new("{?([0-9-]+)\\/?", G_REGEX_OPTIMIZE, G_REGEX_MATCH_DEFAULT, NULL);
    }

    g_autoptr(GMatchInfo) match_info = NULL;
    if (g_regex_match(date_regex, val, G_REGEX_MATCH_DEFAULT, &match_info)) {
      g_autofree gchar *match = g_match_info_fetch(match_info, 1);
      guint64 year = 0;
      guint64 month = 0;
      sscanf(match, "%lu-%lu", &year, &month);

      if (year > 0) {
        g_autofree gchar *fill = g_strnfill(length - strlen("year") + 1, ' ');
        g_string_printf(property, "    year%s= %lu,\n", fill, year);
      }

      if (month > 0) {
        g_autofree gchar *fill = g_strnfill(length - strlen("month") + 1, ' ');
        g_string_append_printf(property, "    month%s= %lu,\n", fill, month);
      }
    } else {
      g_printerr("Could not parse date [%s]\n", val);
      return NULL;
    }
  } else {
    g_autofree gchar *fill = g_strnfill(length - strlen(key) + 1, ' ');
    g_string_printf(property, "    %s%s= %s,\n", key, fill, val);
  }

  return property;
}

gchar *bib_entry_print_type(BIBEntry *entry) {
  if (g_strcmp0(entry->type, "report") == 0) {
    return g_strdup("techreport");
  } else if (g_strcmp0(entry->type, "online") == 0) {
    return g_strdup("misc");
  } else if (g_strcmp0(entry->type, "thesis") == 0) {
    gchar *type = g_hash_table_lookup(entry->properties, "type");
    if (type != NULL && type[0] == 'm') {
      return g_strdup("mastersthesis");
    } else {
      return g_strdup("phdthesis");
    }
  }

  return g_strdup(entry->type);
}

gchar *bib_entry_print_property(gchar *property) {
  if (g_strcmp0(property, "location") == 0) {
    return g_strdup("address");
  } else if (g_strcmp0(property, "journaltitle") == 0) {
    return g_strdup("journal");
  }

  return g_strdup(property);
}

GString *bib_entry_print(BIBEntry *entry, gboolean bibtex) {
  GString *formatted = g_string_sized_new(sizeof(char) * 80 * 7); // ~ 7 lines of 80 char per entry
  if (bibtex) {
    g_autofree gchar *type = bib_entry_print_type(entry);
    g_string_printf(formatted, "@%s{%s,\n", type, entry->key);
  } else {
    g_string_printf(formatted, "@%s{%s,\n", entry->type, entry->key);
  }

  gsize max_length = 0;
  GList *keys = g_hash_table_get_keys(entry->properties);
  GList *printable_keys = NULL;
  gboolean has_doi = false;

  for (GList *k = keys; k != NULL; k = k->next) {
    gchar *key = k->data;

    if ((g_ascii_strcasecmp(key, "keywords") == 0) ||
        (g_ascii_strcasecmp(key, "abstract") == 0) ||
        (g_ascii_strcasecmp(key, "file") == 0)) {
      continue;
    }

    if (g_ascii_strcasecmp(key, "doi") == 0) {
      has_doi = true;
    }

    max_length = MAX(strlen(key), max_length);
    printable_keys = g_list_append(printable_keys, key);
  }

  for (GList *k = printable_keys; k != NULL; k = k->next) {
    gchar *key = k->data;
    gchar *val = g_hash_table_lookup(entry->properties, key);

    if (has_doi && (0 == g_ascii_strcasecmp(key, "issn") ||
                    0 == g_ascii_strcasecmp(key, "isbn") ||
                    0 == g_ascii_strcasecmp(key, "eprint") ||
                    0 == g_ascii_strcasecmp(key, "eprintype") ||
                    0 == g_ascii_strcasecmp(key, "eprintclass") ||
                    0 == g_ascii_strcasecmp(key, "url") ||
                    0 == g_ascii_strcasecmp(key, "urldate"))) {
      continue;
    }

    if (bibtex) {
      g_autofree gchar *bibtex_key = bib_entry_print_property(key);
      g_autoptr(GString) property = bib_property_print(bibtex_key, val, max_length, bibtex);

      if (property == NULL) {
        continue;
      }

      g_string_append_len(formatted, property->str, property->len);
    } else {
      g_autoptr(GString) property = bib_property_print(key, val, max_length, bibtex);
      g_string_append_len(formatted, property->str, property->len);
    }
  }

  g_list_free(keys);
  g_list_free(printable_keys);

  g_string_append(formatted, "}");

  return formatted;
}

GString *bib_entry_list_print(BIBEntryList *list, gboolean bibtex) {
  GString *formatted = g_string_sized_new(sizeof(char) * list->len * 80 * 7); // ~ 7 lines of 80 char per entry

  gchar *last_key = NULL;
  for (gsize i = 0; i < list->len; i++) {
    BIBEntry *entry = g_ptr_array_index(list, i);
    if (g_strcmp0(last_key, entry->key) == 0) {
      g_printerr("Skipping duplicate key %s\n", last_key);
      continue;
    }
    g_autoptr(GString) formatted_entry = bib_entry_print(entry, bibtex);
    g_string_append(formatted, formatted_entry->str);
    g_string_append(formatted, "\n\n");
    last_key = entry->key;
  }

  return formatted;
}
