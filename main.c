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
WebServer *thisServer;
GHashTable *web_modules;
GHashTable *system_services;
GSList *monitors;



bool service_add(char *type, ServiceInfo *service_table)
{
    DEBUG("Adding service %s", type, NULL);

    if(g_hash_table_contains(system_services, type))
    {
        DEBUG("Service was already installed", NULL);
        return FALSE;
    }
    g_hash_table_insert(system_services, type, service_table);
    DEBUG("Service succesfully installed", NULL);
    return TRUE;
}

void service_remove(const char *type)
{
    DEBUG("Removing service %s", type, NULL);

    if(g_hash_table_remove(system_services, type))
    {
        DEBUG("Service %s removed", type, NULL);
    }
    else
    {
        DEBUG("Service %s was to be removed, but was not installed", type, NULL);
    }
}

ServiceInfo *service_get(const char *type)
{
    DEBUG("Getting service %s", type, NULL);
    return g_hash_table_lookup(system_services, type);
}

/**
 * Installs a module in the MyWebsite web server.
 *
 * This function loads the module specified in module_path, constructs a strct WebModule for it, and calls it's install(SoupServer *) function.
 * In case of errors, it will log it (DEBUG) and just return
 * @param module_path A GFile pointer to the module library file
 */
void module_install(GFile *module_path)
{
    char *path = g_file_get_path(module_path);
    DEBUG("Loading module at path %s...", path, NULL);

    WebModule *newModule = malloc(sizeof(WebModule));
    newModule->module = g_module_open(path, G_MODULE_BIND_MASK);
    if(!newModule->module)
    {
        DEBUG("Error loading module: %s", g_module_error(), NULL);
        return;
    }
    DEBUG("Module loaded", NULL);

    DEBUG("Checking interface version...", NULL);
    if(!g_module_symbol(newModule->module, "get_interface_version", (gpointer*)(&newModule->get_interface_version)))
    {
        DEBUG("Error getting symbol 'get_interface_version': %s", g_module_error(), NULL);
        return;
    }
    if(newModule->get_interface_version() != WEB_MODULE_INTERFACE_VERSION)
    {
        DEBUG("Unsupported interface version. Supported: %i, found: %i", WEB_MODULE_INTERFACE_VERSION, newModule->get_interface_version(), NULL);
        return;
    }
    DEBUG("Module interface version: %i", newModule->get_interface_version(), NULL);

    DEBUG("Getting other symbols...", NULL);
    if(!g_module_symbol(newModule->module, "get_name", (gpointer*)(&newModule->get_name)))
    {
        DEBUG("Error getting symbol 'get_name': %s", g_module_error(), NULL);
        return;
    }
    if(!g_module_symbol(newModule->module, "get_version", (gpointer*)(&newModule->get_version)))
    {
        DEBUG("Error getting symbol 'get_version': %s", g_module_error(), NULL);
        return;
    }
    if(!g_module_symbol(newModule->module, "install", (gpointer*)(&newModule->install)))
    {
        DEBUG("Error getting symbol 'install': %s", g_module_error(), NULL);
        return;
    }
    if(!g_module_symbol(newModule->module, "uninstall", (gpointer*)(&newModule->uninstall)))
    {
        DEBUG("Error getting symbol 'uninstall': %s", g_module_error(), NULL);
        return;
    }
    DEBUG("Symbols linked", NULL);

    DEBUG("Module %s, version %i loaded, installing...", newModule->get_name(), newModule->get_version(), NULL);
    if(newModule->install(thisServer))
    {
        DEBUG("Module installed", NULL);
    }
    else
    {
        DEBUG("Error during module installment", NULL);
    }

    g_hash_table_insert(web_modules, path, newModule);
}

/**
 * Uninstalls a module in the MyWebsite web server.
 *
 * This function calls the uninstall(SoupServer *) function of the specified module and closes the module file.
 * @param module_path A GFile pointer to the module library file
 */
void module_uninstall(GFile *module_path)
{
    char *path = g_file_get_path(module_path);

    DEBUG("Getting module struct for module at %s...", path, NULL);
    WebModule *module = (WebModule*)g_hash_table_lookup(web_modules, path);
    if(module == NULL)
    {
        DEBUG("Error: module == NULL", NULL);
        return;
    }
    DEBUG("Module struct for %s (version %i) found, uninstalling...", module->get_name(), module->get_version(), NULL);
    module->uninstall(thisServer);
    DEBUG("Module uninstalled", NULL);
    DEBUG("Closing module...", NULL);
    if(!g_module_close(module->module))
    {
        DEBUG("Error closing module: %s", g_module_error(), NULL);
        return;
    }
    DEBUG("Module closed", NULL);

    DEBUG("Removing module from web_modules...", NULL);
    if(!g_hash_table_remove(web_modules, path))
    {
        DEBUG("Was not found??", NULL);
        return;
    }
    DEBUG("Module removed from web_modules", NULL);

    DEBUG("Cleaning up WebModule struct...", NULL);
    free(module);
    DEBUG("WebModule struct cleaned up", NULL);

    g_free(path);
}

/**
 * This function is used in a loop over all loaded modules to uninstall them.
 *
 * This function will call uninstall(SoupServer *) and close the module for the value.
 * @param key The key used in the GHashTable to reference to this module
 * @param value The pointer to the struct WebModule that defines this module
 * @param user_data Extra data provided by GLib, always NULL in the current implementation
 */
void modules_uninstall(gpointer key,
                      gpointer value,
                      gpointer user_data)
{
    WebModule *module = (WebModule*)value;
    if(module == NULL)
        return;

    DEBUG("Uninstalling module %s (version %i)...", module->get_name(), module->get_version(), NULL);
    module->uninstall(thisServer);
    DEBUG("Module uninstalled", NULL);
    DEBUG("Closing module...", NULL);
    if(!g_module_close(module->module))
    {
        DEBUG("Error closing module: %s", g_module_error(), NULL);
        return;
    }
    DEBUG("Module closed", NULL);

    DEBUG("Cleaning up WebModule struct...", NULL);
    free(module);
    DEBUG("WebModule struct cleaned up", NULL);
}

void file_changed(GFileMonitor *monitor,
                    GFile *file,
                    GFile *other_file,
                    GFileMonitorEvent event,
                    gpointer user_data);

/**
 * @internal
 * A helper function used to be able to print nice debug messages
 * @endinternal
 */
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

/**
 * This either watches for changes in file, or try to install file.
 *
 * If file is a directory, it will add a new GFileMonitor for it and watch it.
 * If file is a normal file, it will try to install it using module_install(GFile *).
 * @param file The file or directory to check
 * @param error Output for the error, if adding fails
 * @return True if succesfully added file and it's sub-directories. False otherwise
 */
bool adddir(GFile *file, GError **error)
{
    //First check if we will need to watch this "file"
    GFileType type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    if(type != G_FILE_TYPE_DIRECTORY)
    {
        //Assume a module
        module_install(file);
        return true;
    }

    gchar *path = g_file_get_parse_name(file);
    DEBUG("\tStarting to watch %s", path, NULL);
    g_free(path);

    GFileMonitor *mon = g_file_monitor(file, G_FILE_MONITOR_NONE, NULL, error);
    if(mon == NULL)
    {
        return false;
    }
    monitors = g_slist_prepend(monitors, mon);
    g_signal_connect(mon, "changed", G_CALLBACK(file_changed), NULL);

    //Add any child sub-directories and HWM files recursively
    DEBUG("\t\tDirectory, adding sub-dirs...", NULL);
    GFileEnumerator *enumer = g_file_enumerate_children(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, error);
    if(enumer == NULL)
    {
        DEBUG("Invalid enumerator?? Error: %s", (*error)->message, NULL);
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

/**
 * This function gets called whenever a watched file or directory changes.
 *
 * If a new directory is created, it will call adddir(GFile *, GError **) to add a watcher for it.
 * If a new file is created, it will try to install it using module_install(GFile *).
 * If a file is deleted, it will try to uninstall it using module_uninstall(GFile *).
 * If a file is changed, it will try to uninstall it using module_uninstall(GFile *) and re-load it using module_install(GFile *).
 */
void file_changed(GFileMonitor *monitor,
                    GFile *file,
                    GFile *other_file,
                    GFileMonitorEvent event,
                    gpointer user_data)
{
    char *msg = decode(event);
    GError *error = NULL;

    DEBUG("Received event %s (code %d), first file \"%s\", second file \"%s\"",
            msg,
            event,
            ((file) ? g_file_get_basename(file) : "--"),
            ((other_file) ? g_file_get_basename(other_file) : "--"),
            NULL);

    if(g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY)
    {
        //New dir
        if(event == 3)
            adddir(file, &error);
    }
    else if(event == 2)
    {
        //File deleted, uninstall
        module_uninstall(file);
    }
    else if(event == 3)
    {
        //File created, install
        module_install(file);
    }
    else if(event == 1)
    {
        //File changed, reinstall
        module_uninstall(file);
        module_install(file);
    }

    g_free(msg);
}



/**
 * This function handles signals by exiting the server.
 */
void signal_handle(int signal_type)
{
    DEBUG("Handling signal %i", signal_type, NULL);

    g_main_loop_quit(loop);
}

#ifdef SIGHUP
/**
 * This function is the handler for the SIGHUP call.
 *
 * It calls signal_handle(int) to handle it)
 */
void signal_hup()
{
    signal_handle(SIGHUP);
    signal(SIGHUP, signal_hup);
}
#endif //SIGHUP

/**
 * This function is the handler for the SIGTERM call.
 *
 * It calls signal_handle(int) to handle it)
 */
void signal_term()
{
    signal_handle(SIGTERM);
    signal(SIGTERM, signal_term);
}

/**
 * This function is the handler for the SIGINT call.
 *
 * It calls signal_handle(int) to handle it)
 */
void signal_int()
{
    signal_handle(SIGINT);
    signal(SIGINT, signal_int);
}

/**
 * This function is the main entry function for the MyWebsite web server.
 *
 * It will initialize the Soup server and GLib, parse the command-line arguments, setup the file monitor for the rootdir, and start everything.
 * After the main loop exits, it will also clear up everything.
 */
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
        DEBUG("Daemonizing...", NULL);
        int frk = fork();
        if(frk < 0)
        {
            g_printerr("Error forking!");
            return 3;
        }
        else if(frk != 0)
        {
            DEBUG("Forked to %i. Exiting parent...", frk, NULL);
            return 0;
        }
        DEBUG("Deamonization complete", NULL);
        //Forked child will continue past here
    }   

    DEBUG("Registering signal handlers...", NULL);
    #ifdef SIGHUP
        signal(SIGHUP, signal_hup);
    #endif //SIGHUP
    signal(SIGTERM, signal_term);
    signal(SIGINT, signal_int);
    DEBUG("Signal handlers registered", NULL);

    DEBUG("Initializing storage...", NULL);
    thisServer = malloc(sizeof(WebServer));
    web_modules = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
    system_services = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    DEBUG("Storage initialized", NULL);

    DEBUG("Initializing WebServer...", NULL);;
    thisServer->add_service = service_add;
    thisServer->remove_service = service_remove;
    thisServer->get_system_service = service_get;
    DEBUG("WebServer initialized", NULL);

    DEBUG("Initializing SoupServer...", NULL);
    thisServer->soupServer = soup_server_new(SOUP_SERVER_PORT, opt_port, NULL);
    if(thisServer->soupServer == NULL)
    {
        g_printerr("Error initializing Soup server. Exiting.\n");
        return 1;
    }
    GValue server_header = G_VALUE_INIT;
    g_value_init(&server_header, G_TYPE_STRING);
    g_value_set_static_string(&server_header, SERVER_HEADER);
    g_object_set_property((GObject*)(thisServer->soupServer), "server-header", &server_header);
    gint port;
    g_object_get(thisServer->soupServer, "port", &port, NULL);
    soup_server_run_async(thisServer->soupServer);
    DEBUG("SOUP Server initialized using port: %i", port, NULL);

    DEBUG("Initializing module installment watcher...", NULL);
    adddir(g_file_new_for_path(opt_rootdir), &error);
    DEBUG("Installment watcher initialized", NULL);

    DEBUG("Initializing main loop...", NULL);
    loop = g_main_loop_new(NULL, FALSE);
    DEBUG("Starting main loop...", NULL);
    g_main_loop_run(loop);
    DEBUG("Main loop exited", NULL);

    DEBUG("Cleaning up file system watcher...", NULL);
    g_slist_free_full(monitors, g_object_unref);
    DEBUG("File system watcher cleaned up", NULL);
    DEBUG("Uninstalling all modules...", NULL);
    g_hash_table_foreach(web_modules, modules_uninstall, NULL);
    DEBUG("All modules uninstalled", NULL);
    DEBUG("Cleaning up SOUP Server...", NULL);
    soup_server_disconnect(thisServer->soupServer);
    DEBUG("SOUP Server cleaned up", NULL);
    DEBUG("Cleaning up storage...", NULL);
    g_hash_table_unref(web_modules);
    free(thisServer);
    DEBUG("Storage cleanedup", NULL);

    return 0;
}
