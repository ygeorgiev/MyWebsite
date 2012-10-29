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
    return "helloworld";
}

const int get_version()
{
    return 1;
}


static const char test_html[] = 
    "<html>"
    "   <head>"
    "       <title>MyWebsite testing page</title>"
    "   </head>"
    "   <body>"
    "       <h1>MyWebsite</h1>"
    "       This website is powered by MyWebsite, an Ansi C custom web server.<br />"
    "       This website is running version: "
    PACKAGE_VERSION
    "       <br />"
    "   </body>"
    "</html>";

static void test_callback(SoupServer *server,
                          SoupMessage *msg,
                          const char *path,
                          GHashTable *query,
                          SoupClientContext *client,
                          gpointer user_data)
{
    DEBUG("test webpage called with path: %s", path, NULL);

    soup_message_set_status(msg, SOUP_STATUS_OK);
    soup_message_set_response(msg, "text/html", SOUP_MEMORY_STATIC, test_html, strlen(test_html));

    DEBUG("test webpage returned", NULL);
}

bool deploy(SoupServer *server)
{   
    soup_server_add_handler(server,
                            "/helloworld",
                            test_callback,
                            NULL,
                            NULL);

    return true;
}

void undeploy(SoupServer *server)
{
    soup_server_remove_handler(server,
                              "/helloworld");
}
