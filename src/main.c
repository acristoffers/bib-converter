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
#include <locale.h>

static void printVersion(void) {
  static const gchar *path = "/me/acristoffers/remove-trash/version";
  GBytes *data = g_resources_lookup_data(path, G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
  gsize size = 0;
  const guint8 *raw_data = g_bytes_get_data(data, &size);
  g_print("%s", raw_data);
}

GString *file_read(const gchar *path, GError **error) {
  g_autoptr(GError) fn_error = NULL;
  g_autoptr(GFile) file = g_file_new_for_commandline_arg(path);
  g_autoptr(GFileInputStream) in = g_file_read(file, NULL, &fn_error);

  if (fn_error != NULL) {
    g_propagate_error(error, fn_error);
    return NULL;
  }

  GString *contents = g_string_new(NULL);
  gchar buffer[4096];
  gssize length;

  while ((length = g_input_stream_read(G_INPUT_STREAM(in), buffer, sizeof(buffer), NULL, &fn_error)) > 0) {
    g_string_append_len(contents, buffer, length);
  }

  if (fn_error != NULL) {
    g_propagate_error(error, fn_error);
    g_string_free(contents, TRUE);
    return NULL;
  }

  return contents;
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "en_US.UTF-8");

  struct options options = parse_options(argc, argv);

  if (options.version) {
    printVersion();
    return 0;
  }

  if (g_strv_length(options.rest) < 1) {
    g_printerr("Missing bib file in arguments.\n");
    return 1;
  }

  const gchar *path = options.rest[0];

  g_autoptr(GError) error = NULL;
  g_autoptr(GString) contents = file_read(path, &error);

  if (error != NULL) {
    g_printerr("Error reading file: %s", error->message);
    return 1;
  }

  g_autoptr(BIBEntryList) entries = bib_parse(contents, &error);

  if (error != NULL) {
    g_printerr("Error: %s\n", error->message);
    return 1;
  }

  g_autoptr(GString) formatted = bib_entry_list_print(entries, options.bibtex);
  g_print("%s\n", formatted->str);

  g_strfreev(options.rest);
  free_regex();

  return 0;
}
