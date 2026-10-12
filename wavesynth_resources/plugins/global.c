#include "pluginutil.h"

Parameter parameters[MAX_NUM_PARAMS];
int init = 0;

Parameter *int_params[MAX_NUM_PARAMS];
int num_int_params = 0;

int srand_called = 0;

int pindex = 0;

#ifdef LOGGING
char log_buffer[1024] = "";
FILE *log_file_handle = NULL;
#endif
