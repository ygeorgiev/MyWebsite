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
#include "web_module.h"

GMainLoop *loop;
SoupServer *soupServer;
GHashTable *web_modules;
GSList *monitors;



void module_deploy(GFile *module_path)
{
    char *path = g_file_get_path(module_path);
    DEBUG("Loading module at path %s...", path);

    WebModule *newModule = malloc(sizeof(WebModule));
    newModule->module = g_module_open(path, G_MODULE_BIND_MASK);
    if(!newModule->module)
    {
        DEBUG("Error loading module: %s", g_module_error());
        return;
    }
    DEBUG("Module loaded");

    DEBUG("Checking interface version...");
    if(!g_module_symbol(newModule->module, "get_interface_version", (gpointer*)(&newModule->get_interface_version)))
    {
        DEBUG("Error getting symbol 'get_interface_version': %s", g_module_error());
        return;
    }
    if(newModule->get_interface_version() != WEB_MODULE_INTERFACE_VERSION)
    {
        DEBUG("Unsupported interface version. Supported: %i, found: %i", WEB_MODULE_INTERFACE_VERSION, newModule->get_interface_version());
        return;
    }
    DEBUG("Module interface version: %i", newModule->get_interface_version());

    DEBUG("Getting other symbols...");
    if(!g_module_symbol(newModule->module, "get_name", (gpointer*)(&newModule->get_name)))
    {
        DEBUG("Error getting symbol 'get_name': %s", g_module_error());
        return;
    }
    if(!g_module_symbol(newModule->module, "get_version", (gpointer*)(&newModule->get_version)))
    {
        DEBUG("Error getting symbol 'get_version': %s", g_module_error());
        return;
    }
    if(!g_module_symbol(newModule->module, "deploy", (gpointer*)(&newModule->deploy)))
    {
        DEBUG("Error getting symbol 'deploy': %s", g_module_error());
        return;
    }
    if(!g_module_symbol(newModule->module, "undeploy", (gpointer*)(&newModule->undeploy)))
    {
        DEBUG("Error getting symbol 'undeploy': %s", g_module_error());
        return;
    }
    DEBUG("Symbols linked");

    DEBUG("Module %s, version %i loaded, deploying...", newModule->get_name(), newModule->get_version());
    if(newModule->deploy(soupServer))
    {
        DEBUG("Module deployed");
    }
    else
    {
        DEBUG("Error during module deployment");
    }

    g_hash_table_insert(web_modules, path, newModule);
}

void module_undeploy(GFile *module_path)
{
    char *path = g_file_get_path(module_path);

    DEBUG("Getting module struct for module at %s...", path);
    WebModule *module = (WebModule*)g_hash_table_lookup(web_modules, path);
    if(module == NULL)
    {
        DEBUG("Error: module == NULL");
        return;
    }
    DEBUG("Module struct for %s (version %i) found, undeploying...", module->get_name(), module->get_version());
    module->undeploy(soupServer);
    DEBUG("Module undeployed");
    DEBUG("Closing module...");
    if(!g_module_close(module->module))
    {
        DEBUG("Error closing module: %s", g_module_error());
        return;
    }
    DEBUG("Module closed");

    g_free(path);
}


void file_changed(GFileMonitor *monitor,
                    GFile *file,
                    GFile *other_file,
                    GFileMonitorEvent event,
                    gpointer user_data);
char *
decode (GFileMonitorEvent ev)
{
    char *fmt = (char*)g_malloc0 (1024);
    int caret = 0;

#define dc(x) \
case G_FILE_MONITOR_EVENT_##x: \
strcat(fmt, #x); \
caret += strlen(#x); \
fmt[caret] = '\0'; \
break;

    switch (ev) {
        dc(CHANGED);
        dc(CHANGES_DONE_HINT);
        dc(DELETED);
        dc(CREATED);
        dc(ATTRIBUTE_CHANGED);
        dc(PRE_UNMOUNT);
        dc(UNMOUNTED);
        dc(MOVED);
    }
#undef dc

    return fmt;
}

bool adddir(GFile *file, GError **error)
{
    //First check if we will need to watch this "file"
    GFileType type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    if(type != G_FILE_TYPE_DIRECTORY)
    {
        //Assume a module
        module_deploy(file);
        return true;
    }

    gchar *path = g_file_get_parse_name(file);
    DEBUG("\tStarting to watch %s", path);
    g_free(path);

    GFileMonitor *mon = g_file_monitor(file, G_FILE_MONITOR_NONE, NULL, error);
    if(mon == NULL)
    {
        return false;
    }
    monitors = g_slist_prepend(monitors, mon);
    g_signal_connect(mon, "changed", G_CALLBACK(file_changed), NULL);

    //Add any child sub-directories and HWM files recursively
    DEBUG("\t\tDirectory, adding sub-dirs...");
    GFileEnumerator *enumer = g_file_enumerate_children(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, error);
    if(enumer == NULL)
    {
        DEBUG("Invalid enumerator?? Error: %s", (*error)->message);
        return false;
    }
    GFileInfo *childInfo;
    while ((childInfo = g_file_enumerator_next_file (enumer,
                                               NULL, error)) != NULL)
    {
        /* Do something with the file info */
        GFile *child = g_file_get_child(file, g_file_info_get_name(childInfo));
        gboolean childResult = adddir(child, error);
        g_object_unref(child);
        g_object_unref(childInfo);
        if(!childResult)
        {
            return false;
        }
    }
    g_file_enumerator_close(enumer, NULL, NULL);
    g_object_unref(enumer);
    return true;
}

void file_changed(GFileMonitor *monitor,
                    GFile *file,
                    GFile *other_file,
                    GFileMonitorEvent event,
                    gpointer user_data)
{
    #define fn(x) ((x) ? g_file_get_basename (x) : "--")
    char *msg = decode(event);
    GError *error = NULL;

    DEBUG("Received event %s (code %d), first file \"%s\", second file \"%s\"",
            msg,
            event,
            fn(file),
            fn(other_file));

    if(g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY)
    {
        //New dir
        if(event == 3)
            adddir(file, &error);
    }
    else if(event == 2)
    {
        //File deleted, undeploy
        module_undeploy(file);
    }
    else if(event == 3)
    {
        //File created, deploy
        module_deploy(file);
    }
    else if(event == 1)
    {
        //File changed, redeploy
        module_undeploy(file);
        module_deploy(file);
    }

    g_free(msg);
    #undef fn
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
    gchar *opt_rootdir = ".";
    gboolean opt_daemonize = FALSE;
    gboolean opt_debug = FALSE;
    gboolean opt_version = FALSE;
    GOptionContext *opt_context;
    GError *error;
    GOptionEntry opt_entries[] = 
        {
            {"rootdir", 'r', 0, G_OPTION_ARG_STRING, &opt_rootdir, "Handler directory", NULL},
            {"daomonize", 'D', 0, G_OPTION_ARG_NONE, &opt_daemonize, "Daemonize     (fork)", NULL},
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

    if(opt_daemonize)
    {   
        DEBUG("Daemonizing...");
        int frk = fork();
        if(frk < 0)
        {
            g_printerr("Error forking!");
            return 3;
        }
        else if(frk != 0)
        {
            DEBUG("Forked to %i. Exiting parent...", frk);
            return 0;
        }
        DEBUG("Deamonization complete");
        //Forked child will continue past here
    }   

    DEBUG("Registering signal handlers...");
    #ifdef SIGHUP
        signal(SIGHUP, signal_hup);
    #endif //SIGHUP
    signal(SIGTERM, signal_term);
    signal(SIGINT, signal_int);
    DEBUG("Signal handlers registered");

    DEBUG("Initializing storage...");
    web_modules = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    DEBUG("Storage initialized");

    DEBUG("Initializing SOUP Server...");
    soupServer = soup_server_new(SOUP_SERVER_PORT, opt_port, NULL);
    GValue server_header = G_VALUE_INIT;
    g_value_init(&server_header, G_TYPE_STRING);
    g_value_set_static_string(&server_header, SERVER_HEADER);
    g_object_set_property((GObject*)soupServer, "server-header", &server_header);
    gint port;
    g_object_get(soupServer, "port", &port, NULL);
    soup_server_run_async(soupServer);
    DEBUG("SOUP Server initialized using port: %i", port);

    DEBUG("Initializing module deployment watcher...");
    adddir(g_file_new_for_path(opt_rootdir), &error);
    DEBUG("Deployment watcher initialized");

    DEBUG("Initializing main loop...");
    loop = g_main_loop_new(NULL, FALSE);
    DEBUG("Starting main loop...");
    g_main_loop_run(loop);
    DEBUG("Main loop exited");

    DEBUG("Cleaning up file system watcher...");
    g_slist_free_full(monitors, g_object_unref);
    DEBUG("File system watcher cleaned up");
    DEBUG("Undeploying all modules...");

    DEBUG("All modules undeployed");
    DEBUG("Cleaning up SOUP Server...");
    soup_server_disconnect(soupServer);
    DEBUG("SOUP Server cleaned up");

    return 0;
}
