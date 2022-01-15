#ifndef COPY_FROM_TO_H
#define COPY_FROM_TO_H

#include "gnome.h"

int get_num_at_end(const char *search)
{
    char c = search[strlen(search) - 1];
    if (c >= '0' && c <= '3')
        return c - '0';
    return -1;
}

void copy_from_to(int from, int to)
{
    double param_copies[100];
    int param_copies_idx = 0;
    FOR_EACH_PARAMETER(i)
    {
        /*if (parameters[i].name[0] == '[')
            continue;*/
        int num = get_num_at_end(parameters[i].name);
        if (num == from)
        {
            if (to == 0)
            {
                char new_name[64];
                strcpy(new_name, parameters[i].name);
                new_name[strlen(new_name) - 1] = '0';
                new_parameter(new_name, parameters[i].value);
            }
            else
                param_copies[param_copies_idx++] = parameters[i].value;
        }
    }

    if (to == 0)
        return;

    param_copies_idx = 0;
    FOR_EACH_PARAMETER(i)
    {
        /*if (parameters[i].name[0] == '[')
            continue;*/
        int num = get_num_at_end(parameters[i].name);
        if (num == to)
        {
            parameters[i].value = param_copies[param_copies_idx++];
        }
    }
}

#endif