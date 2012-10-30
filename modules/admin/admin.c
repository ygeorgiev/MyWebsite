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

#include <services/Configuration.h>

ServiceInfoConfiguration *config;

int get_interface_version()
{
    return WEB_MODULE_INTERFACE_VERSION;
}

const char *get_name()
{
    return "admin";
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
    "       <h1>MyWebsite - ADMIN</h1>"
    "       This website is powered by MyWebsite, an Ansi C custom web server.<br />"
    "       This website is running version: "
    PACKAGE_VERSION
    "       <br />"
    "   </body>"
    "</html>";


static void admin_callback(SoupServer *server,
                          SoupMessage *msg,
                          const char *path,
                          GHashTable *query,
                          SoupClientContext *client,
                          gpointer user_data)
{
    soup_message_set_status(msg, SOUP_STATUS_OK);
    soup_message_set_response(msg, "text/html", SOUP_MEMORY_STATIC, test_html, strlen(test_html));
}

char *admin_auth_callback (SoupAuthDomain *domain,
                           SoupMessage *msg,
                           const char *username,
                           gpointer user_data)
{
    if(strcmp(username, "patrick") == 0)
    {
        return soup_auth_domain_digest_encode_password("patrick", "MyWebsite Admin", "dublin");
    }
    else
    {
        return NULL;
    }
}

SoupAuthDomain *admin_domain = NULL;

bool install(WebServer *server)
{
    ServiceInfo *configInfo = server->get_system_service(SERVICE_TYPE_CONFIGURATION);
    if(configInfo == NULL)
    {
        DEBUG("Configuration system service is not provided, admin module cannot initialize", NULL);
        return FALSE;
    }
    config = (ServiceInfoConfiguration*)configInfo->get_service();

    admin_domain = soup_auth_domain_digest_new(SOUP_AUTH_DOMAIN_REALM, "MyWebsite Admin",
                                               SOUP_AUTH_DOMAIN_DIGEST_AUTH_CALLBACK, admin_auth_callback,
                                               SOUP_AUTH_DOMAIN_DIGEST_AUTH_DATA, NULL,
                                               SOUP_AUTH_DOMAIN_ADD_PATH, "/admin",
                                               NULL);

    soup_server_add_auth_domain(server->soupServer, admin_domain);

    soup_server_add_handler(server->soupServer,
                            "/admin",
                            admin_callback,
                            NULL,
                            NULL);

    return true;
}

void uninstall(WebServer *server)
{
    soup_server_remove_handler(server->soupServer,
                              "/admin");
    soup_server_remove_auth_domain(server->soupServer, admin_domain);
}
