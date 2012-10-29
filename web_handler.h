//This file gives the headers of all init-functions

#ifndef WEB_H_INCLUDED
#define WEB_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gio/gio.h>
#include <libsoup/soup.h>

#include <config.h>


struct WebHandler
{
    GModule *module;
    void (*deploy)(SoupServer *server);
    void (*undeploy)(SoupServer *server);
};


#endif //WEB_H_INCLUDED
