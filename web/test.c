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
    DEBUG("test webpage called with path: %s", path);

    soup_message_set_status(msg, SOUP_STATUS_OK);
    soup_message_set_response(msg, "text/html", SOUP_MEMORY_STATIC, test_html, strlen(test_html));

    DEBUG("test webpage returned");
}

void web_init_test(SoupServer *server)
{   
    DEBUG("Initializing test web handler...");

    soup_server_add_handler(server,
                            "/test",
                            test_callback,
                            NULL,
                            NULL);

    DEBUG("Test web handler initialized");
}
