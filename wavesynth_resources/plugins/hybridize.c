#include "pluginutil.h"
#include <windows.h>

typedef int (*exec_plugin_func)(char *, size_t);

// Runs another plugin DLL from the same directory as this one on the given
// parameter buffer. Returns the number of output strings or -1 on failure.
static int exec_sub_plugin(const char *dll_name, char *str, size_t sz)
{
    HMODULE self;
    char path[MAX_PATH];
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR)&exec_sub_plugin, &self) ||
        !GetModuleFileNameA(self, path, MAX_PATH))
        return -1;
    char *last_slash = strrchr(path, '\\');
    if (!last_slash || strlen(path) + strlen(dll_name) >= MAX_PATH)
        return -1;
    strcpy(last_slash + 1, dll_name);

    LOG("Path is %s", path);

    HMODULE dll = LoadLibraryA(path);
    if (!dll)
        return -1;
    exec_plugin_func sub_exec_plugin = (exec_plugin_func)GetProcAddress(dll, "exec_plugin");
    int count = sub_exec_plugin ? sub_exec_plugin(str, sz) : -1;
    FreeLibrary(dll);
    return count;
}

__declspec(dllexport) int exec_plugin(char *str, size_t sz)
{
    INIT;
    LOG("Initialized plugin");

    static Parameter p_orig[MAX_NUM_PARAMS];
    memcpy(p_orig, parameters, sizeof(parameters));

    int type = P(type);
    double hybridize_amount = P(hybridize_amount);

    LOG("Type = %d", type);

    const char *dll_name;
    if (type == 0)
        dll_name = "pad.dll";
    else if (type == 1)
        dll_name = "lead.dll";
    else if (type == 2)
        dll_name = "bass.dll";
    else
        return -1;

    // The input parameters are already read, so the buffer can be handed over
    if (exec_sub_plugin(dll_name, str, sz) < 0)
        return -1;

    LOG("Sub plugin call done");

    read_ipc_data_str(str, sz);

    LOG("new init done");
    for (int i = 0; p_orig[i].name[0]; i++)
    {
        // Skip waveforms
        if (!strcmp(p_orig[i].name, "Bank_1") || !strcmp(p_orig[i].name, "Bank_2") || !strcmp(p_orig[i].name, "Bank_3") ||
            !strcmp(p_orig[i].name, "Patch_1") || !strcmp(p_orig[i].name, "Patch_2") || !strcmp(p_orig[i].name, "Patch_3"))
            {
                char intname[64] = "*";
                strcat(intname, p_orig[i].name);
                Parameter *p = get_parameter_ptr(intname);
                strcpy(p->name, p_orig[i].name);
                p->value = p_orig[i].value;
                continue;
            }
        Parameter *p = get_parameter_ptr(p_orig[i].name);
        if (!p)
            continue;
        double gen_val = p->value;
        double orig_val = p_orig[i].value;
        double new_val = orig_val * (1 - hybridize_amount) + (gen_val + orig_val) * hybridize_amount / 2;
        p->value = new_val;
    }
    LOG("Finalizing work");
    FINALIZE;
}
