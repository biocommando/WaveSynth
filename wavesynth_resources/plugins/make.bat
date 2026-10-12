set CC=gcc -static -shared global.c -O2 -DBUILD_DLL
if "%1"=="tcc" (set CC=.\tcc\tcc.exe global.c)

%CC% bass.c -o bass.dll
%CC% copyfromto.c -o copy_from_to.dll
%CC% cyclesynthstack.c -o cycle_synth_stack.dll
%CC% delaysync.c -o delaysync.dll
%CC% hybridize.c -o hybridize.dll
%CC% init_all.c -o init_all.dll
%CC% lead.c -o lead.dll
%CC% lfo_sync_wrapper.c -o lfo_sync_wrapper.dll
%CC% orchestral.c -o orchestral.dll
%CC% pad.c -o pad.dll
%CC% randomflanger.c -o random_flanger.dll
%CC% randompatch.c -o random_patch.dll
%CC% reseedrandomseq.c -o reseed_random_seq.dll
%CC% transpose.c -o transpose.dll
%CC% unison.c -o unison.dll