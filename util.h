#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

class Util
{
    static std::string resolveWorkDir(bool workOutsideOfRoot)
    {
        static std::string workDir;
        if (workDir != "")
            return workDir;
        // work out the resource directory
        // first we get the DLL path from windows API
        extern void *hInstance;
#ifdef UNICODE
        wchar_t workDirWc[1024];
        GetModuleFileName((HMODULE)hInstance, workDirWc, 1024);
        char workDirC[1024];
        wcstombs(workDirC, workDirWc, 1024);
#else
        char workDirC[1024];
        GetModuleFileName((HMODULE)hInstance, workDirC, 1024);
#endif

        workDir.assign(workDirC);

        // let's get rid of the DLL file name
        auto posBslash = workDir.find_last_of('\\');
        std::string dllName = "unknown";
        if (posBslash != std::string::npos)
        {
            dllName = workDir.substr(posBslash + 1);
            workDir = workDir.substr(0, posBslash);
        }

        // dllDir = workDir;

        // Let's find out the actual directory we want to work in
        if (workOutsideOfRoot)
        {
            auto workDirSpec = workDir + "\\" + dllName + ".txt";
            std::ifstream f;
            f.open(workDirSpec);
            std::getline(f, workDirSpec);
            f.close();
            if (workDirSpec != "")
                workDir = workDirSpec;
        }
        return workDir;
    }

public:
    static std::vector<std::string> splitString(const std::string &s, char c)
    {
        std::vector<std::string> ret;
        int pos = 0;
        do
        {
            auto pos0 = pos;
            pos = s.find(c, pos);
            if (pos != std::string::npos)
            {
                ret.push_back(s.substr(pos0, pos - pos0));
                pos++;
            }
            else
            {
                ret.push_back(s.substr(pos0));
            }
        } while (pos != std::string::npos);
        return ret;
    }

    static std::string getWorkDir(bool workOutsideOfRoot = true)
    {
        return resolveWorkDir(workOutsideOfRoot);
    }

    static int getSelection(float value, int numOptions)
    {
        return value * numOptions * 0.999;
    }

    static float getSelectionValue(int index, int numOptions)
    {
        if (numOptions == 0)
            return 0;
        return ((float)index + 0.5f) / (float)numOptions;
    }
};