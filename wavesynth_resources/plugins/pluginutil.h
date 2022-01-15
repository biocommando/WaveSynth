#ifndef PLUGIN_UTIL_H
#define PLUGIN_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define MAX_NUM_PARAMS 1000
#define LAST_PARAM (MAX_NUM_PARAMS - 1)

typedef struct
{
    char name[64];
    double value;
} Parameter;

Parameter parameters[MAX_NUM_PARAMS];
int init = 0;

#define FOR_EACH_PARAMETER(iteration_idx) \
    for (int iteration_idx = 0; parameters[iteration_idx].name[0]; iteration_idx++)

Parameter *get_parameter_ptr(const char *name)
{
    if (!init || !name || !name[0])
        return NULL;
    FOR_EACH_PARAMETER(i)
    {
        if (!strcmp(name, parameters[i].name))
        {
            LOG_TRACE(STR2("Returning requested parameter", name));
            return &parameters[i];
        }
    }
    LOG_TRACE(STR2("Failed to find requested parameter", name));
    return NULL;
}

void new_parameter(const char *name, double value)
{
    if (get_parameter_ptr(name) == NULL && strlen(name) < 64)
    {
        int i;
        for (i = 0; parameters[i].name[0]; i++)
        {
        }
        if (i < LAST_PARAM)
        {
            strcpy(parameters[i].name, name);
            parameters[i].value = value;
            parameters[i + 1].name[0] = 0;
            LOG_TRACE(STR2("Created new parameter", name));
        }
    }
}

#define PS(name) \
    ((get_parameter_ptr(name) == NULL) ? &parameters[LAST_PARAM] : get_parameter_ptr(name))->value

#define P(name) PS(#name)

void read_ipc_data(const char *filename)
{
    parameters[LAST_PARAM].value = 0;
    parameters[0].name[0] = 0;
    FILE *f = fopen(filename, "r");
    int param_count = 0;
    init = 1;
    while (!feof(f))
    {
        char buf[100];
        fgets(buf, 100, f);
        char name[100];
        double value;
        if (sscanf(buf, "%s %lf", name, &value) == 2)
        {
            new_parameter(name, value);
        }
    }
    fclose(f);
}

void save_ipc_data(const char *filename)
{
    if (!init)
        return;
    FILE *f = fopen(filename, "w");
    FOR_EACH_PARAMETER(i)
    {
        fprintf(f, "%s %lf\n", parameters[i].name, parameters[i].value);
    }
    fclose(f);
}

#define MAIN_ARGS int argc, char **argv

#define INIT      \
    if (argc < 2) \
        return 1; \
    read_ipc_data(argv[1])

#define FINALIZE            \
    save_ipc_data(argv[1]); \
    return 0

#define PLUGIN()                           \
    void plugin_implementation_function(); \
    int main(MAIN_ARGS)                    \
    {                                      \
        LOG(STR2("Init plugin", argv[0]));       \
        INIT;                              \
        LOG(STR1("Plugin init done"));       \
        plugin_implementation_function();  \
        LOG(STR1("Plugin executed"));        \
        FINALIZE;                          \
    }                                      \
    void plugin_implementation_function()

#endif
