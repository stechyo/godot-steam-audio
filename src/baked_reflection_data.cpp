#include "baked_reflection_data.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SteamAudioBakedReflectionData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_serialized_data", "data"), &SteamAudioBakedReflectionData::set_serialized_data);
    ClassDB::bind_method(D_METHOD("get_serialized_data"), &SteamAudioBakedReflectionData::get_serialized_data);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "serialized_data"), "set_serialized_data", "get_serialized_data");
}

SteamAudioBakedReflectionData::SteamAudioBakedReflectionData() {
}

SteamAudioBakedReflectionData::~SteamAudioBakedReflectionData() {
}

void SteamAudioBakedReflectionData::set_serialized_data_internal(IPLbyte *data) {
    serialized_data.resize(sizeof(data));
    for (size_t i = 0; i < sizeof(data); ++i) {
        serialized_data[i] = data[i];
    }

    Error result = ResourceSaver::get_singleton()->save(this, get_path(), ResourceSaver::FLAG_COMPRESS);
    if (result != OK) {
        UtilityFunctions::push_error("Failed to save the baked reflection data resource. Error code: " + String::num_int64(result));
    }
}

void SteamAudioBakedReflectionData::set_serialized_data(const PackedByteArray& p_data) {
    serialized_data = p_data;
}

PackedByteArray SteamAudioBakedReflectionData::get_serialized_data() const {
    return serialized_data;
}
