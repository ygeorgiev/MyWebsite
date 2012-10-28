#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include <config.h>
#include "utils.h"

int main(int argc, char **argv)
{
    g_type_init();

    gboolean opt_debug = FALSE;
    gboolean opt_version = FALSE;
    GOptionContext *opt_context;
    GError *error;
    GOptionEntry opt_entries[] = 
        {
            {"version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, "Show version", NULL},
            {"debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug, "Enable debugging", NULL},
            {NULL},
        };

    error = NULL;
    opt_context = g_option_context_new("My Website Server");
    g_option_context_add_main_entries(opt_context, opt_entries, NULL);
    if(!g_option_context_parse(opt_context, &argc, &argv, &error))
    {
        g_printerr("Error parsing options: %s\n", error->message);
        return 1;
    }
    set_debug(opt_debug);
    if(opt_version)
    {
        printf("My Website Server version %s\n", PACKAGE_VERSION);
        return 0;
    }

    printf("Hello world");
    return 0;
}
