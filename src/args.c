#include "internal.h"

struct options parse_options(int argc, char **argv) {
  struct options o = {0};

  GOptionEntry entries[] = {
      {"bibtex",           'b', 0, G_OPTION_ARG_NONE,           &o.bibtex,  "Output for bibtex instead of biblatex", ""},
      {"version",          'v', 0, G_OPTION_ARG_NONE,           &o.version, "Show version",                          ""},
      {G_OPTION_REMAINING, 0,   0, G_OPTION_ARG_FILENAME_ARRAY, &o.rest,    "File",                                  ""},
      G_OPTION_ENTRY_NULL
  };

  g_autoptr(GError) error = NULL;
  g_autoptr(GOptionContext) context = NULL;

  static const gchar *desc = "BIB-FILE";
  context = g_option_context_new(desc);
  g_option_context_add_main_entries(context, entries, NULL);

  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  return o;
}
