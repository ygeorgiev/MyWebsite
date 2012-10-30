#ifndef SERVICE_CONFIGURATION_H_INCLUDED
#define SERVICE_CONFIGURATION_H_INCLUDED

#define SERVICE_TYPE_CONFIGURATION "configuration"

enum _ServiceConfigurationValueType
{
    SERVICE_CONFIGURATION_VALUE_TYPE_INT,
    SERVICE_CONFIGURATION_VALUE_TYPE_DOUBLE,
    SERVICE_CONFIGURATION_VALUE_TYPE_STRING
};
typedef enum _ServiceConfigurationValueType ServiceConfigurationValueType;

struct _ServiceInfoConfiguration
{
    void (*add_configuration_variable)(const char *name, ServiceConfigurationValueType type);
    void (*update_string)(const char *name, const char *new_value);
    void (*update_double)(const char *name, const double new_value);
    void (*update_int)(const char *name, const int new_value);
    const char *(*get_string)(const char *name);
    const double (*get_double)(const char *name);
    const double (*get_int)(const char *name);
};
typedef struct _ServiceInfoConfiguration ServiceInfoConfiguration;

#endif //SERVICE_CONFIGURATION_H_INCLUDED
