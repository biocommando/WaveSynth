// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa

// parameters (optional): skip_synth_1 skip_synth_2 skip_synth_3 skip_all_synths skip_global
// 1 = true, 0 = false

include $/legacy.js

inline sub init_all

	def skip_synth_1
	def skip_synth_2
	def skip_synth_3
	def skip_all_synths
	def skip_global

	if skip_all_synths == 0
		for index in 1 to 3
			resolve_question_mark skip_synth_?
			if skip_synth_? == 0
%				Bank_? = 0
%				Patch_? = 0
%				Loop? = 0.5
%				CuePos? = 0
%				Octave? = 0.5
%				Detune? = 0.5
%				FCut_? = 1
%				FRes_? = 0
%				Wavshp? = 0
%				Attack? = 0
%				Decay? = 0
%				Sust.? = 1
%				Tail.? = 0
%				E2Cut_? = 0
%				LFOShp? = 0
%				LFOFrq? = 0
%				LFO2Pt? = 0
%				LFO2Cu? = 0
%				LFO2Fl? = 0
%				LFO2Lv? = 0
%				Pan_? = 0.5
%				Level_? = 1
			endif
		loop
	endif

	if skip_global == 0
		Level = 0.5
		Unison = 0
		UDetune = 0
		DelayTm = 0
		DlyFeed = 0
		DlyWet = 0
		OscSeq = 0
		SoftClp = 0
		FlngFrq = 0
		FlngDly = 0
		FlngDpt = 0
		FlngWet = 0
		LfoFreq = 0
		LfoType = 0
		L2Pan = 0
		L2Level = 0
		L2Delay = 0
		L2Flang = 0
		LowCut = 0
		L2LFOs = 0
		L2Clip = 0
		LStereo = 0
	endif
return