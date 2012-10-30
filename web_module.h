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

struct _ServiceInfo
{
    const char *(*get_name)();
    const int (*get_version)();

    void *(*get_service)();
};
typedef struct _ServiceInfo ServiceInfo;

struct _WebServer
{
    SoupServer *soupServer;
    bool (*add_service)(char *type, ServiceInfo *serviceInfo);
    void (*remove_service)(const char *type);
    ServiceInfo *(*get_system_service)(const char *type);
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
