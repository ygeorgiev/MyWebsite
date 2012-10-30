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
    bool (*is_module_loaded)(const char *module_name);
    bool (*add_service)(char *type, ServiceInfo *serviceInfo);
    void (*remove_service)(const char *type);
    ServiceInfo *(*get_system_service)(const char *type);
};
typedef struct _WebServer WebServer;

enum _InstallResult
{
    INSTALL_RESULT_OK,
    INSTALL_RESULT_DEPENDENCY_NOT_LOADED,
    INSTALL_RESULT_FAILED,
    INSTALL_RESULT_COULD_NOT_LOAD
};
typedef enum _InstallResult InstallResult;

struct _WebModule
{
    GModule *module;
    const int (*get_interface_version)();
    const char *(*get_name)();
    const int (*get_version)();
    InstallResult (*install)(WebServer *server);
    void (*uninstall)(WebServer *server);
};
typedef struct _WebModule WebModule;

#endif //WEB_MODULE_H_INCLUDED
