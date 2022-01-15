#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Log global parameter definitions

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_ERROR 2
#ifndef LOG_FILTER
#define LOG_FILTER ""
#endif
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// Logging state

char log_buffer[1024] = "";
FILE *log_file_handle = NULL;
const char log_lev_map[][6] = {"TRACE", "INFO ", "ERROR"};

// Actual logging implementation

void log_write(const char *file, int line, int level)
{
    if (LOG_LEVEL > level || (LOG_FILTER[0] && strcmp(LOG_FILTER, file)))
        return;
    if (!log_file_handle)
        return;
    char pad[] = "                ";
    pad[sizeof(pad) - strlen(file) > 0 ? sizeof(pad) - strlen(file) : 0] = 0;
    fprintf(log_file_handle, "[%s%s |%4d] %s %s\n",
            pad, file, line, log_lev_map[level % 3], log_buffer);
#ifdef CONSOLE_LOGGING
    printf("%s\n", log_buffer);
#endif
}

void log_lifecycle(const char *ipc_file)
{
    if (ipc_file)
    {
        if (!log_file_handle)
        {
            strcpy(log_buffer, ipc_file);
            for (int i = strlen(log_buffer) - 1; log_buffer[i] != '\\' && log_buffer[i] != '/'; i--)
                log_buffer[i] = 0;
            strcat(log_buffer, "log.txt");
            log_file_handle = fopen(log_buffer, "a");
            fprintf(log_file_handle, "--- Plugin init ---\n");
            fprintf(log_file_handle, "Log init: level='%s', filter='%s'\n",
                    log_lev_map[LOG_LEVEL % 3], LOG_FILTER);
        }
    }
    else
    {
        fclose(log_file_handle);
        log_file_handle = NULL;
    }
}

// Logging macros, use LOG_TRACE, LOG and LOG_ERROR for logging in source files

#define DO_LOG(level, ...)                    \
    do                                        \
    {                                         \
        sprintf(log_buffer, __VA_ARGS__);     \
        log_write(__FILE__, __LINE__, level); \
    } while (0)

#define LOG_TRACE(...) DO_LOG(LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG(...) DO_LOG(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_ERROR(...) DO_LOG(LOG_LEVEL_ERROR, __VA_ARGS__)

#endif