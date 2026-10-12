#pragma once
#include "util.h"
#include "aeffguieditor.h"

// Loads bitmaps from files (same approach as in FuzzOfDoom). hInstance points to the host exe
// (as in clapwrap), so the bitmaps can't be loaded from the plugin's resources.
class CBitmapLoader
{
public:
    // relativePath is relative to the resource directory (wavesynth_resources)
    static CBitmap *load(const char *relativePath)
    {
        std::string s = Util::getWorkDir(true) + "\\" + relativePath;
        std::wstring ws(s.size(), L'#');
        mbstowcs(&ws[0], s.c_str(), s.size());
        auto bmp = Gdiplus::Bitmap::FromFile(ws.c_str(), false);
        auto cbmp = new CBitmap(bmp);
        delete bmp;
        return cbmp;
    }
};
