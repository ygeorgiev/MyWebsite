#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include <config.h>
#include <utils.h>
#include <web_module.h>

int get_interface_version()
{
    return WEB_MODULE_INTERFACE_VERSION;
}

const char *get_name()
{
    return "blank";
}

const int get_version()
{
    return 1;
}

static void blank_callback(SoupServer *server,
                          SoupMessage *msg,
                          const char *path,
                          GHashTable *query,
                          SoupClientContext *client,
                          gpointer user_data)
{
}

InstallResult install(WebServer *server)
{   
    soup_server_add_handler(server->soupServer,
                            "",
                            test_callback,
                            NULL,
                            NULL);

    return INSTALL_RESULT_OK;
}

void uninstall(WebServer *server)
{
    soup_server_remove_handler(server->soupServer,
                              "");
}
