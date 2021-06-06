#include "audio_handle.h"
#include "Player.h"

namespace nd {
AudioHandle::AudioHandle(bool isSound, bool isSpatial)
	: m_is_sound(isSound),
	  m_is_spatial(isSpatial) {}

bool AudioHandle::isPlaying() { return Sounder::get().isPlaying(m_handle); }

void AudioHandle::updateSpatialData(const SpatialData& data)
{
	//ASSERT(m_is_spatial, "Changing SpatialData on sound which is not spatial")
	if (m_spatial_data == data)
		return;
	m_spatial_data = data;
	if (m_is_playing)
		Sounder::get().updateSpatialData(m_handle, data);
}

SoundHandle::SoundHandle()
	:
	AudioHandle(true) {}

MusicHandle::MusicHandle()
	:
	AudioHandle(false) {}

void AudioHandle::open(const std::string& filePath)
{
	m_is_playing = false;
	m_file_path = filePath;
}

void AudioHandle::pause()
{
	if (m_is_paused)
		return;
	m_is_paused = true;
	SoundAssignment as;
	as.type = SoundAssignment::PAUSE;
	as.id = m_handle;
	as.timeToChangeVolume = 0;
	Sounder::get().submit(as);
	m_is_playing = false;
}

void AudioHandle::play(float fadeTime)
{
	if (m_is_paused)
	{
		SoundAssignment as;
		as.type = SoundAssignment::PLAY;
		as.id = m_handle;
		as.timeToChangeVolume = fadeTime;
		Sounder::get().submit(as);
		m_is_paused = false;
	}
	else
		m_handle = Sounder::get().playAudio(m_file_path, m_is_sound, m_volume, m_pitch, m_loop, fadeTime, m_spatial_data);
	m_is_playing = true;
}

void AudioHandle::stop(float fadeTime)
{
	SoundAssignment as;
	as.type = SoundAssignment::CLOSE;
	as.id = m_handle;
	as.timeToChangeVolume = fadeTime;
	Sounder::get().submit(as);
	m_is_playing = false;
}

void AudioHandle::setVolume(float volume, float fadeTime)
{
	m_volume = volume;
	if (m_is_playing)
	{
		SoundAssignment as;
		as.type = SoundAssignment::FADE;
		as.id = m_handle;
		as.volume = m_volume;
		as.pitch = m_pitch;
		as.timeToChangeVolume = fadeTime;
		Sounder::get().submit(as);
	}
}

void AudioHandle::setPitch(float f, float fadeTime)
{
	m_pitch = f;
	if (m_is_playing)
	{
		SoundAssignment as;
		as.type = SoundAssignment::FADE;
		as.id = m_handle;
		as.volume = m_volume;
		as.pitch = m_pitch;
		as.timeToChangePitch = fadeTime;
		Sounder::get().submit(as);
	}
}
}
