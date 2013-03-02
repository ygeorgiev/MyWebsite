/**
 * Copyright (c) 2012, 2013,  Patrick Uiterwijk <puiterwijk@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of Patrick Uiterwijk nor the
 *      names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Patrick Uiterwijk BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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

InstallResult install(WebServer *server)
{
    ServiceInfo *configInfo = server->get_system_service(SERVICE_TYPE_CONFIGURATION);
    if(configInfo == NULL)
    {
        DEBUG("Configuration system service is not provided, admin module cannot initialize", NULL);
        return INSTALL_RESULT_DEPENDENCY_NOT_LOADED;
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

    return INSTALL_RESULT_OK;
}

void uninstall(WebServer *server)
{
    soup_server_remove_handler(server->soupServer,
                              "/admin");
    soup_server_remove_auth_domain(server->soupServer, admin_domain);
}
