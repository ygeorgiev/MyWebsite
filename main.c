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
#include "web.h"

GMainLoop *loop;
SoupServer *soupServer;

void initSoup()
{
    //Add all serverhandlers and auth filters to soupServer
    web_init_test(soupServer);

}


void signal_handle(int signal_type)
{
    DEBUG("Handling signal %i", signal_type);

    g_main_loop_quit(loop);
}

#ifdef SIGHUP
void signal_hup()
{
    signal_handle(SIGHUP);
    signal(SIGHUP, signal_hup);
}
#endif //SIGHUP

void signal_term()
{
    signal_handle(SIGTERM);
    signal(SIGTERM, signal_term);
}

void signal_int()
{
    signal_handle(SIGINT);
    signal(SIGINT, signal_int);
}


int main(int argc, char **argv)
{
    g_type_init();

    int opt_port = 8080;
    gboolean opt_debug = FALSE;
    gboolean opt_version = FALSE;
    GOptionContext *opt_context;
    GError *error;
    GOptionEntry opt_entries[] = 
        {
            {"port", 'p', 0, G_OPTION_ARG_INT, &opt_port, "Set port (default: 8080)", NULL},
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

    DEBUG("Registering signal handlers...");
    #ifdef SIGHUP
        signal(SIGHUP, signal_hup);
    #endif //SIGHUP
    signal(SIGTERM, signal_term);
    signal(SIGINT, signal_int);
    DEBUG("Signal handlers registered");

    DEBUG("Initializing SOUP Server...");
    soupServer = soup_server_new(SOUP_SERVER_PORT, opt_port, NULL);
    soup_server_run_async(soupServer);
    DEBUG("SOUP Server initialized");

    DEBUG("Initializing paths and modules in the SOUP Server...");
    initSoup();
    DEBUG("SOUP Server ready for use");

    DEBUG("Initializing main loop...");
    loop = g_main_loop_new(NULL, FALSE);
    DEBUG("Starting main loop...");
    g_main_loop_run(loop);
    DEBUG("Main loop exited");

    DEBUG("Cleaning up SOUP Server...");
    soup_server_disconnect(soupServer);
    DEBUG("SOUP Server cleaned up");

    return 0;
}
