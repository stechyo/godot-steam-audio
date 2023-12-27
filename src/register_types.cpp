#include "register_types.hpp"

#include "config.hpp"
#include "geometry.hpp"
#include "listener.hpp"
#include "material.hpp"
#include "player.hpp"
#include "server.hpp"
#include "stream.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void init_ext(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE && p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
		return;
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<SteamAudioStreamPlayback>();
		ClassDB::register_class<SteamAudioStream>();
		ClassDB::register_class<SteamAudioListener>();
		ClassDB::register_class<SteamAudioGeometry>();
		ClassDB::register_class<SteamAudioMaterial>();
		ClassDB::register_class<SteamAudioConfig>();
		ClassDB::register_class<SteamAudioPlayer>();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		auto sa = memnew(SteamAudioServer);
		GDREGISTER_CLASS(SteamAudioServer);
	}
}

void uninit_ext(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE && p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
		return;
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT init_extension(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(init_ext);
	init_obj.register_terminator(uninit_ext);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
