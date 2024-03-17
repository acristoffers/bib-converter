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

#include <string.h>
#include <tree_sitter/api.h>

G_DEFINE_AUTOPTR_CLEANUP_FUNC(TSParser, ts_parser_delete)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(TSTree, ts_tree_delete)

extern const TSLanguage *tree_sitter_biber(void);

static GRegex *regex = NULL;

void free_regex(void) {
  g_regex_unref(regex);
}

gchar *remove_symbols(const gchar *str) {
  gchar *an = g_malloc0_n(strlen(str), sizeof(char));
  gsize j = 0;
  for (gsize i = 0; i < strlen(str); i++) {
    gchar c = str[i];
    if (g_ascii_isalnum(c)) {
      an[j++] = c;
    }
  }
  return an;
}

guint64 parse_year(const gchar *year) {
  g_autofree gchar *y = remove_symbols(year);
  guint64 num = g_ascii_strtoull(y, NULL, 10);
  return num;
}

#define hash_month_expr(A, B, C) ((A << 16) | (B << 8) | C)
#define hash_month(MONTH) hash_month_expr(g_ascii_tolower(MONTH[0]), g_ascii_tolower(MONTH[1]), g_ascii_tolower(MONTH[2]))
#define hash_month_case(A, B, C, D) \
  case hash_month_expr(A, B, C):    \
    return D

guint64 parse_month(const gchar *month) {
  g_autofree gchar *m = remove_symbols(month);

  if (g_ascii_isdigit(m[0])) {
    guint64 num = g_ascii_strtoull(m, NULL, 10);
    if (num > 0 && num < 13) {
      return num;
    } else {
      return 0;
    }
  }

  if (strlen(m) < 3) {
    return 0;
  }

  switch (hash_month(m)) {
    hash_month_case('j', 'a', 'n', 1);
    hash_month_case('f', 'e', 'b', 2);
    hash_month_case('m', 'a', 'r', 3);
    hash_month_case('a', 'p', 'r', 4);
    hash_month_case('m', 'a', 'y', 5);
    hash_month_case('j', 'u', 'n', 6);
    hash_month_case('j', 'u', 'l', 7);
    hash_month_case('a', 'u', 'g', 8);
    hash_month_case('s', 'e', 'p', 9);
    hash_month_case('o', 'c', 't', 10);
    hash_month_case('n', 'o', 'v', 11);
    hash_month_case('d', 'e', 'c', 12);
    default:
      return 0;
  }
}

gchar *ts_node_text(TSNode node, const GString *contents) {
  uint32_t start = ts_node_start_byte(node);
  uint32_t end = ts_node_end_byte(node);
  size_t length = end - start;

  if (length == 0) {
    return NULL;
  }

  if (regex == NULL) {
    regex = g_regex_new("[\\s\\n]+", G_REGEX_OPTIMIZE | G_REGEX_NEWLINE_ANYCRLF, G_REGEX_MATCH_DEFAULT, NULL);
  }

  g_autofree gchar *normalized = g_utf8_normalize(contents->str + start, length, G_NORMALIZE_ALL);
  gchar *replaced = g_regex_replace_literal(regex, normalized, -1, 0, " ", G_REGEX_MATCH_DEFAULT, NULL);
  return replaced;
}

gchar *bib_parse_entry_to_biblatex_property_name(gchar *name) {
  if (g_strcmp0(name, "address") == 0) {
    return g_strdup("location");
  } else if (g_strcmp0(name, "journal") == 0) {
    return g_strdup("journaltitle");
  }

  return g_strdup(name);
}

BIBEntry *bib_parse_entry(TSNode node, const GString *contents, GError **error) {
  BIBEntry *entry = bib_entry_create();
  guint64 year = 0;
  guint64 month = 0;

  for (gsize i = 0; i < ts_node_named_child_count(node); i++) {
    TSNode child = ts_node_named_child(node, i);
    const char *type = ts_node_type(child);

    switch (type[0]) {
      case 'n': { // name
        g_autofree gchar *type = ts_node_text(child, contents);
        if (g_ascii_strcasecmp(type, "phdthesis") == 0) {
          entry->type = g_strdup("thesis");
          g_hash_table_insert(entry->properties, g_strdup("type"), g_strdup("phdthesis"));
        } else if (g_ascii_strcasecmp(type, "mastersthesis") == 0) {
          entry->type = g_strdup("thesis");
          g_hash_table_insert(entry->properties, g_strdup("type"), g_strdup("mathesis"));
        } else {
          entry->type = g_utf8_strdown(type, -1);
        }
        continue;
      }
      case 'k': { // key
        g_autofree gchar *key = ts_node_text(child, contents);
        entry->key = g_utf8_strdown(key, -1);
        continue;
      }
      case 'f': { // field
        TSNode key = ts_node_named_child(child, 0);
        TSNode value = ts_node_named_child(child, 1);
        g_autofree gchar *key_text_raw = ts_node_text(key, contents);
        gchar *key_text = g_utf8_strdown(key_text_raw, -1);
        gchar *value_text = ts_node_text(value, contents);
        if (g_ascii_strcasecmp(key_text, "year") == 0) {
          year = parse_year(value_text);
          g_free(key_text);
          g_free(value_text);
          continue;
        } else if (g_ascii_strcasecmp(key_text, "month") == 0) {
          month = parse_month(value_text);
          g_free(key_text);
          g_free(value_text);
          continue;
        } else {
          gchar *biblatex_name = bib_parse_entry_to_biblatex_property_name(key_text);
          g_hash_table_insert(entry->properties, biblatex_name, value_text);
        }
        continue;
      }
      default:
        continue;
    }
  }

  if (year > 0) {
    if (month > 0) {
      gchar *date = g_strdup_printf("{%lu-%lu}", year, month);
      g_hash_table_insert(entry->properties, g_strdup("date"), date);
    } else {
      gchar *date = g_strdup_printf("{%lu}", year);
      g_hash_table_insert(entry->properties, g_strdup("date"), date);
    }
  }

  return entry;
}

static gint sort_entries(gconstpointer a, gconstpointer b) {
  const BIBEntry *entry1 = *((BIBEntry **)a);
  const BIBEntry *entry2 = *((BIBEntry **)b);
  return g_ascii_strcasecmp(entry1->key, entry2->key);
}

BIBEntryList *bib_parse(const GString *bibfile, GError **error) {
  g_autoptr(TSParser) parser = NULL;
  g_autoptr(TSTree) tree = NULL;
  g_autoptr(GError) fn_error = NULL;

  parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_biber());
  tree = ts_parser_parse_string(parser, NULL, bibfile->str, bibfile->len);

  TSNode root_node = ts_tree_root_node(tree);
  BIBEntryList *entries = bib_entry_list_create();

  for (gsize i = 0; i < ts_node_named_child_count(root_node); i++) {
    TSNode node = ts_node_named_child(root_node, i);
    if (g_strcmp0(ts_node_type(node), "entry") == 0) {
      BIBEntry *entry = bib_parse_entry(node, bibfile, &fn_error);

      if (fn_error != NULL) {
        bib_entry_list_free(entries);
        g_propagate_error(error, fn_error);
        return NULL;
      }

      g_ptr_array_add(entries, entry);
    }
  }

  g_ptr_array_sort(entries, sort_entries);

  return entries;
}
