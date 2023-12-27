#ifndef STEAM_AUDIO_STREAM_H
#define STEAM_AUDIO_STREAM_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "steam_audio.hpp"
#include <phonon.h>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/templates/safe_refcount.hpp>

using namespace godot;

class SteamAudio;

class SteamAudioStream : public AudioStream {
	GDCLASS(SteamAudioStream, AudioStream)
	friend class SteamAudioStreamPlayback;

protected:
	static void _bind_methods();

public:
	SteamAudioStream();
	~SteamAudioStream();

	Ref<AudioStreamPlayback> _instantiate_playback() const override;
};

class SteamAudioStreamPlayback : public AudioStreamPlayback {
	GDCLASS(SteamAudioStreamPlayback, AudioStreamPlayback);
	friend class SteamAudioStream;

private:
	Ref<AudioStream> stream;
	Ref<AudioStreamPlayback> stream_playback;
	LocalSteamAudioState local_state;

	std::atomic<bool> is_local_state_init;

	void init_local_state();

protected:
	static void _bind_methods();

public:
	SteamAudioStreamPlayback();
	~SteamAudioStreamPlayback();

	void set_parent(Node3D *node);
	void set_cfg(SteamAudioSourceConfig cfg);

	virtual int32_t _mix(AudioFrame *buffer, double rate_scale,
			int32_t frames) override;
	int play_stream(const Ref<AudioStream> &p_stream, float p_from_offset,
			float p_volume_db, float p_pitch_scale);
	void _start(double from_pos) override;
	void _stop() override;
};

#endif
