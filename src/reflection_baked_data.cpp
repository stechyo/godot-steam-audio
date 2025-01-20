#include "reflection_baked_data.hpp"
#include <godot_cpp/core/class_db.hpp>
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

void SteamAudioBakedReflectionData::set_serialized_data(const PackedByteArray& p_data) {
    serialized_data = p_data;
}

PackedByteArray SteamAudioBakedReflectionData::get_serialized_data() const {
    return serialized_data;
}
