#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LOG_FILE
struct logevent
{
    char str[1024];
};

#define LOG(evt) do_log(evt, __FILE__, __LINE__)

#define STR1(str) get_logevent(str)

#define STR(evt, str2) get_logevent_s(evt, str2)
#define STR2(str, str2) get_logevent_s(STR1(str), str2)
#define INT(evt, i) get_logevent_i(evt, i)
#define DBL(evt, d) get_logevent_d(evt, d)

void do_log(struct logevent evt, const char *file, int line)
{
    static int init = 0;
    FILE *f = fopen(LOG_FILE, "a");
    if (!init)
        fprintf(f, "--- Plugin init ---\n");
    init = 1;
    fprintf(f, "line %d: file %s: %s\n", line, file, evt.str);
    fclose(f);
}

struct logevent get_logevent(const char *str)
{
    struct logevent evt;
    strcpy(evt.str, str);
    return evt;
}

struct logevent get_logevent_s(struct logevent evt, const char *str)
{
    strcat(evt.str, " ");
    strcat(evt.str, str);
    return evt;
}

struct logevent get_logevent_i(struct logevent evt, int i)
{
    char val[20];
    sprintf(val, " %d", i);
    strcat(evt.str, val);
    return evt;
}

struct logevent get_logevent_d(struct logevent evt, double d)
{
    char val[20];
    sprintf(val, " %lf", d);
    strcat(evt.str, val);
    return evt;
}

#ifdef ENABLE_LOG_TRACE
#define LOG_TRACE(evt) LOG(evt)
#else
#define LOG_TRACE(a)
#endif

#else
#define LOG(a)
#define LOG_TRACE(a)
#define STR1(str)

#define STR(evt, str2)
#define STR2(str, str2)
#define INT(evt, i)
#define DBL(evt, d)
#endif
#endif