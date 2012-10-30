#ifndef WEB_MODULE_H_INCLUDED
#define WEB_MODULE_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include <config.h>


#define WEB_MODULE_INTERFACE_VERSION 1

struct _WebServer
{
    SoupServer *soupServer;
    bool (*add_service)(char *type, void *service_table);
    void (*remove_service)(char *type);
    void *(*get_system_service)(char *type);
};
typedef struct _WebServer WebServer;

struct _WebModule
{
    GModule *module;
    const int (*get_interface_version)();
    const char *(*get_name)();
    const int (*get_version)();
    bool (*install)(WebServer *server);
    void (*uninstall)(WebServer *server);
};
typedef struct _WebModule WebModule;

#endif //WEB_MODULE_H_INCLUDED
