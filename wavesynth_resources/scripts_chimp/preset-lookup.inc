include @/lib/sub_extensions.js

inline sub get_number_of_presets
    notify "preset_lookup", "preset_count"
    // The number of presets now available in variable
    // preset_count
return


sub load_preset(load_preset_number)
    notify "preset_lookup", "load_preset"
    // The parameters loaded to "load_xxxxxx" variables.
    // Wave pack available in variable "load_wave_pack"
return