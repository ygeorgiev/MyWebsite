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
    return "doesntexist";
}

const int get_version()
{
    return 1;
}


static const char doesntexist_html[] = 
    "<html>"
    "   <head>"
    "       <title>Page not found</title>"
    "   </head>"
    "   <body>"
    "       <h1>The specified page could not be found</h1>"
    "       Please navigate to the <a href='/'>Home</a> and try to find the page you searched for again"
    "   </body>"
    "</html>";

static void doesntexist_callback(SoupServer *server,
                          SoupMessage *msg,
                          const char *path,
                          GHashTable *query,
                          SoupClientContext *client,
                          gpointer user_data)
{
    DEBUG("Nonexistent page called: %s", path);

    soup_message_set_status(msg, SOUP_STATUS_OK);
    soup_message_set_response(msg, "text/html", SOUP_MEMORY_STATIC, doesntexist_html, strlen(doesntexist_html));
}

bool deploy(SoupServer *server)
{   
    DEBUG("Deploying doesntexist web module...");

    soup_server_add_handler(server,
                            NULL,
                            doesntexist_callback,
                            NULL,
                            NULL);

    DEBUG("Doesntexist web module deployed");

    return true;
}

void undeploy(SoupServer *server)
{
    DEBUG("Undeploying doesntexist web module...");

    soup_server_remove_handler(server,
                              NULL);

    DEBUG("Doesntexist web module undeployed");
}
