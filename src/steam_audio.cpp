#include "steam_audio.hpp"
#include "config.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void SteamAudio::log(GodotSteamAudioLogLevel lvl, const char *str) {
	if (lvl < SteamAudioConfig::log_level) {
		return;
	}

	if (lvl < log_error) {
		UtilityFunctions::print("[godot-steam-audio] ", str);
	} else {
		UtilityFunctions::push_error("[godot-steam-audio] ", str);
	}
}
