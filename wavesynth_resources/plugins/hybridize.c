#include "pluginutil.h"

int main(MAIN_ARGS)
{
    INIT;
    LOG("Initialized plugin %s", argv[0]);

    char path[2048];
    strcpy(path, argv[1]);
    for (int i = strlen(path) - 1; path[i] != '\\'; i--)
    {
        path[i] = 0;
    }
    
    LOG("Base path = %s", path);

    Parameter p_orig[MAX_NUM_PARAMS];
    memcpy(p_orig, parameters, sizeof(parameters));

    int type = P(type);
    double hybridize_amount = P(hybridize_amount);

    LOG("Type = %d", type);

    if (type == 0)
        strcat(path, "pad.exe");
    else if (type == 1)
        strcat(path, "lead.exe");
    else if (type == 2)
        strcat(path, "bass.exe");
    else
        return -1;

    strcat(path, " ");
    strcat(path, argv[1]);

    LOG("Path is %s", path);

    if (system(path) != 0)
        return -1;

    LOG("Sys call done");

    INIT;

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