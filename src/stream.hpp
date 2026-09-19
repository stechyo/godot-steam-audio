#ifndef STEAM_AUDIO_STREAM_H
#define STEAM_AUDIO_STREAM_H

#include "godot_cpp/classes/ref.hpp"
#include "player.hpp"
#include <phonon.h>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/templates/safe_refcount.hpp>
#include <atomic>
#include <mutex>
#include <unordered_set>

using namespace godot;

class SteamAudio;

class SteamAudioStream : public AudioStream {
	GDCLASS(SteamAudioStream, AudioStream)
	friend class SteamAudioStreamPlayback;
	Ref<AudioStream> stream;

protected:
	static void _bind_methods();

public:
	SteamAudioStream();
	~SteamAudioStream();

	Ref<AudioStreamPlayback> _instantiate_playback() const override;
	void set_stream(Ref<AudioStream> p_stream);
	Ref<AudioStream> get_stream();

	SteamAudioPlayer *parent = nullptr;
	std::shared_ptr<SteamAudioPlayerHandle> handle;
};

class SteamAudioStreamPlayback : public AudioStreamPlayback {
	GDCLASS(SteamAudioStreamPlayback, AudioStreamPlayback);
	friend class SteamAudioStream;

private:
	Ref<AudioStream> stream;
	Ref<AudioStreamPlayback> stream_playback;

	std::atomic<bool> is_active{false};
	// guards `stream_playback` swaps against the audio thread (see _mix)
	std::mutex sp_mux;

protected:
	static void _bind_methods();

public:
	SteamAudioStreamPlayback();
	~SteamAudioStreamPlayback();

	// Exit crash: at exit Godot mixed a SteamAudioStreamPlayback AFTER this library
	// unloaded -- the call into the freed class table jumped to 0xfeeefeee (main thread, AudioServer::_mix_step).
	// Every live playback is registered; on unload `drain` stops them all and waits for the mixer to let go.
	static std::mutex registry_mux;
	static std::unordered_set<SteamAudioStreamPlayback *> registry;
	static std::atomic<bool> shutting_down;
	static int drain(int max_ms);
	static int live_count() { std::lock_guard<std::mutex> g(registry_mux); return int(registry.size()); }
	static std::atomic<int> made, destroyed;

	void set_stream(Ref<AudioStream> p_stream);
	Ref<AudioStreamPlayback> get_stream_playback();

	virtual int32_t _mix(AudioFrame *buffer, float rate_scale, int32_t frames) override;
	int play_stream(const Ref<AudioStream> &p_stream, float p_from_offset,
			float p_volume_db, float p_pitch_scale);
	void _start(double from_pos) override;
	void _stop() override;
	bool _is_playing() const override;

	SteamAudioPlayer *parent = nullptr;
	std::shared_ptr<SteamAudioPlayerHandle> handle;
};

#endif
