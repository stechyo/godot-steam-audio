#ifndef GODOT_STEAM_AUDIO_REFLECTION_BAKED_DATA_HPP
#define GODOT_STEAM_AUDIO_REFLECTION_BAKED_DATA_HPP

#include <godot_cpp/classes/resource.hpp>
#include <phonon.h>

namespace godot {

class SteamAudioBakedReflectionData : public Resource {
    GDCLASS(SteamAudioBakedReflectionData, Resource)

private:
    PackedByteArray serialized_data;
    
protected:
    static void _bind_methods();

public:
    SteamAudioBakedReflectionData();
    ~SteamAudioBakedReflectionData();

    void set_serialized_data_internal(IPLbyte *data);
    void set_serialized_data(const PackedByteArray& p_data);
    PackedByteArray get_serialized_data() const;
};

}

#endif // GODOT_STEAM_AUDIO_BAKED_REFLECTION_BAKED_DATA_HPP
