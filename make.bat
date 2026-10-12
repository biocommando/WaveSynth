@echo off
setlocal
cd /d "%~dp0"

set CLAP_INCLUDE=../../clap/include
set VSTGUI_DIR=../../clapwrap/VSTGui
set DEFINES=-DWIN32 -DWINDOWS -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS
rem to log host calls into wavesynth-clap.log in the temp dir:
Set EXTRA_FLAGS=-DCLAP_DEBUG_LOG
set CXXFLAGS=-O2 -std=c++17 -I%CLAP_INCLUDE% -I%VSTGUI_DIR% %DEFINES% %EXTRA_FLAGS%
rem VSTGui is compiled without optimization like in FuzzOfDoom. It has undefined behaviour
rem (e.g. "CRect viewSize = pV->getViewSize (viewSize);" in vstgui.cpp) that gcc -O2 turns into
rem garbage rects: the GUI then draws only the background colour.
set VSTGUI_FLAGS=-trigraphs -I%VSTGUI_DIR% -DWIN32 -DWINDOWS

if not exist build mkdir build

call "%~dp0prebuild.bat"

rem VSTGui only needs to be rebuilt when it changes. Delete build\*.o to force a rebuild.
if not exist build\vstgui.o (
    g++ -c %VSTGUI_DIR%/vstgui.cpp -o build/vstgui.o %VSTGUI_FLAGS% || exit /b 1
    g++ -c %VSTGUI_DIR%/vstcontrols.cpp -o build/vstcontrols.o %VSTGUI_FLAGS% || exit /b 1
    g++ -c %VSTGUI_DIR%/aeffguieditor.cpp -o build/aeffguieditor.o %VSTGUI_FLAGS% || exit /b 1
)

g++ -shared -static -o wavesynth.clap %CXXFLAGS% ^
    ClapPlugin.cpp WaveSynth.cpp EditorGui.cpp CChangeEvents.cpp ^
    BasicCompressor.cpp BasicDelay.cpp BasicFilter.cpp BasicOscillator.cpp ^
    PredictableRandom.cpp PresetLoader.cpp WavePlayer.cpp ExecutePlugin.cpp ^
    Flanger.cpp IniFileReader.cpp MacroCache.cpp MinimalParameter.cpp ^
    MS20Filter.cpp Saturation.cpp strrpl.cpp WaveSynthSettings.cpp ^
    build/vstgui.o build/vstcontrols.o build/aeffguieditor.o ^
    -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -lmsimg32 || exit /b 1

if not "%1"=="install" exit /b 0
set CLAP_DIR=C:\Program Files\Common Files\CLAP
xcopy /Y .\wavesynth.clap "%CLAP_DIR%" || exit /b 1
rem The plugin finds its resources via <plugin file name>.txt that contains the resource directory path
if not exist "%CLAP_DIR%\wavesynth.clap.txt" (
    echo %~dp0wavesynth_resources>"%CLAP_DIR%\wavesynth.clap.txt"
)
