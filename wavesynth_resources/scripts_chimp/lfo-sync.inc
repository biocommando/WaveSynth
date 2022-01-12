// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa

// input parameters: 
// %frequency
// %noteDivision (1/noteDivision notes)
// output: %output, new frequency as knob value

sub lfo_sync

	script_lfosync_noteLength = 60 / tempo / noteDivision * 4
	// to Hz (the range is 0.01 to 5.01 Hz)
	script_lfosync_freq = (frequency * 5) + 0.01
	script_lfosync_length = (1 / script_lfosync_freq) / script_lfosync_noteLength + 0.4999
	script_lfosync_length = _truncate_
	script_lfosync_length = script_lfosync_length * script_lfosync_noteLength
	if script_lfosync_length < 0.001
		script_lfosync_length = script_lfosync_noteLength
	endif
	script_lfosync_freq = 1 / script_lfosync_length
	output = script_lfosync_freq - 0.01
	output = output * 0.2

return
