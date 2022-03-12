#ifndef PLUGIN_UTIL_H
#define PLUGIN_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LOGGING
#include "log.h"
#else
#define LOG_TRACE(...)
#define LOG(...)
#define LOG_ERROR(...)
#define log_lifecycle(f)
#endif

#define MAX_NUM_PARAMS 1000
#define LAST_PARAM (MAX_NUM_PARAMS - 1)

#define STR_LENGTH 64

typedef struct
{
    char name[STR_LENGTH];
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
            LOG_TRACE("Returning requested parameter %s = %lf", name, parameters[i].value);
            return &parameters[i];
        }
    }
    LOG_TRACE("Failed to find requested parameter: %s", name);
    return NULL;
}

int get_string_parameter(const char *name, char *value)
{
    if (!init || !name)
        return -1;
    LOG_TRACE("Get string param '%s'", name);
    FOR_EACH_PARAMETER(i)
    {
        if (parameters[i].name[0] == '$')
        {
            int found = 1;
            int j;
            for (j = 0; name[j]; j++)
            {
                if (name[j] != parameters[i].name[j + 1])
                {
                    found = 0;
                    break;
                }
            }
            if (found && j == (int)parameters[i].value)
            {
                strncpy(value, &parameters[i].name[j + 1], STR_LENGTH);
                LOG_TRACE("Found string parameter '%s': '%s'", name, value);
                return i;
            }
        }
    }
    LOG_TRACE("Failed to find requested string parameter: '%s'", name);
    return -1;
}

void new_parameter(const char *name, double value)
{
    if (get_parameter_ptr(name) == NULL && strlen(name) < STR_LENGTH)
    {
        int i;
        for (i = 0; parameters[i].name[0]; i++)
        {
        }
        if (i < LAST_PARAM)
        {
            strncpy(parameters[i].name, name, STR_LENGTH);
            parameters[i].value = value;
            parameters[i + 1].name[0] = 0;
            LOG_TRACE("Created new parameter %s", name);
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

void save_ipc_data(char **argv)
{
    if (!init)
        return;
    FILE *f = fopen(argv[1], "w");
    FOR_EACH_PARAMETER(i)
    {
        fprintf(f, "%s %lf\n", parameters[i].name, parameters[i].value);
    }
#ifndef NO_DEBUG_VARIABLES
    fprintf(f, "#debug_data_program_name %s\n", argv[0]);
    fprintf(f, "#debug_data_ipc_file_full_path %s\n", argv[1]);
#endif
    fclose(f);
}

#define MAIN_ARGS int argc, char **argv

#define INIT                \
    if (argc < 2)           \
        return 1;           \
    log_lifecycle(argv[1]); \
    read_ipc_data(argv[1])

#define FINALIZE            \
    save_ipc_data(argv); \
    log_lifecycle(NULL);    \
    return 0

#define PLUGIN()                             \
    void plugin_implementation_function();   \
    int main(MAIN_ARGS)                      \
    {                                        \
        INIT;                                \
        LOG("Plugin init done %s", argv[0]); \
        plugin_implementation_function();    \
        LOG("Plugin executed");              \
        FINALIZE;                            \
    }                                        \
    void plugin_implementation_function()

#endif
