#include "steam_audio.hpp"
#include "config.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void SteamAudio::log(GodotSteamAudioLogLevel lvl, const char *str) {
	if (lvl < SteamAudioConfig::log_level) {
		return;
	}

	switch (lvl) {
		case log_error:
			UtilityFunctions::push_error("[godot-steam-audio] ", str);
			return;
		case log_warn:
			UtilityFunctions::push_warning("[godot-steam-audio] ", str);
			return;
		default:
			UtilityFunctions::print("[godot-steam-audio] ", str);
			return;
	}
}
