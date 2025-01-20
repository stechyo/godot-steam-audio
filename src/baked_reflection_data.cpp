#include "baked_reflection_data.hpp"
#include "server.hpp""

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

void SteamAudioBakedReflectionData::set_data(IPLSerializedObject serialized_object) {
    auto data = iplSerializedObjectGetData(serialized_object);
    serialized_data.resize(iplSerializedObjectGetSize(serialized_object));
    for (size_t i = 0; i < sizeof(data); ++i) {
        serialized_data[i] = data[i];
    }

    Error result = ResourceSaver::get_singleton()->save(this, get_path(), ResourceSaver::FLAG_COMPRESS);
    if (result != OK) {
        UtilityFunctions::push_error("Failed to save the baked reflection data resource. Error code: " + String::num_int64(result));
    }
}

IPLSerializedObject SteamAudioBakedReflectionData::get_data() {
    IPLSerializedObject serialized_object{};
    IPLSerializedObjectSettings settings = {serialized_data.ptrw(), serialized_data.size()};
    IPLContext context = SteamAudioServer::get_singleton()->get_global_state(false)->ctx;
    iplSerializedObjectCreate(context, &settings, &serialized_object);
    return serialized_object;
}

void SteamAudioBakedReflectionData::set_serialized_data(const PackedByteArray& p_data) {
    serialized_data = p_data;
}

PackedByteArray SteamAudioBakedReflectionData::get_serialized_data() const {
    return serialized_data;
}
