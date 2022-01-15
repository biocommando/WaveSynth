#ifndef GNOME_H
#define GNOME_H

#include "pluginutil.h"
#include <time.h>

int pindex = 0;

void qmarkrpl(char *out, const char *in)
{
    int i;
    strcpy(out, in);
    for (i = 0; out[i]; i++)
    {
        if (out[i] == '?')
            out[i] = '0' + pindex;
    }
    LOG_TRACE("Resolved %s notation to %s", in, out);
}

Parameter *get_pqmark_ptr(const char *name)
{
    char buf[20];
    qmarkrpl(buf, name);
    return get_parameter_ptr(buf);
}

#define PQ(name) \
    ((get_pqmark_ptr(name) == NULL) ? &parameters[LAST_PARAM] : get_pqmark_ptr(name))->value

int srand_called = 0;
double random()
{
    if (!srand_called)
    {
        srand((int)time(NULL));
        srand_called = 1;
    }
    return (double)(rand() % 10000) / 10000.0;
}

Parameter *int_params[MAX_NUM_PARAMS];
int num_int_params = 0;

void param_as_integer(const char *name)
{
    int_params[num_int_params] = get_parameter_ptr(name);
    LOG_TRACE("Marked param %s as integer", name);
    num_int_params++;
}

#define FOR_EACH_PSET \
    for (pindex = 1; pindex <= 3; pindex++)

void params_as_integer(const char *namex)
{
    FOR_EACH_PSET
    {
        char buf[20];
        qmarkrpl(buf, namex);
        param_as_integer(buf);
    }
}

void mark_interger_params()
{
    for (int i = 0; i < num_int_params; i++)
    {
        if (int_params[i]->name[0] != '*')
        {
            char buf[64];
            strcpy(buf, int_params[i]->name);
            sprintf(int_params[i]->name, "*%s", buf);
            LOG_TRACE("Integer param '%s' saved with new name '%s'", buf, int_params[i]->name);
        }
    }
}

#endif