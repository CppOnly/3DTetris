#pragma once
#include "stdafx.h"

class AudioManager final {
public:
	AudioManager();
	~AudioManager();

	bool InitXAudio2();

	void PlayMainAudio();
	void StopMainAudio();

	void PlayWinAudio();
	void StopWinAudio();

	void PlayLoseAudio();
	void StopLoseAudio();

	void PlayDropAudio();

private:
	bool BuildMainAudio();
	bool BuildWinAudio();
	bool BuildLoseAudio();
	bool BuildDropAudio();

	bool LoadWAVFromFile(TCHAR* _strFileName, WAVEFORMATEXTENSIBLE &_wfx, XAUDIO2_BUFFER &_buffer, bool _isLoop = false);
	bool FindChunk(HANDLE _hFile, DWORD _chunkID, DWORD &_chunkSize, DWORD &_chunkOffset);
	bool ReadChunkData(HANDLE _hFile, void* _buffer, DWORD _bufferSize, DWORD _bufferOffset);

private:
	Microsoft::WRL::ComPtr<IXAudio2> m_xAudio2 = nullptr;
	IXAudio2MasteringVoice* pMasterVoice = nullptr;

	std::array<IXAudio2SourceVoice*, 4> pAudioes = { nullptr };

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	std::array<XAUDIO2_BUFFER, 4> buffer = { 0 };

	enum AudioType {
		MAIN = 0, WIN = 1, LOSE = 2, DROP = 3
	};

private:
	static bool m_isCreate;
#pragma region _Forbidden Constructor_
private:
	AudioManager(const AudioManager& _rhs) = delete;
	AudioManager& operator=(const AudioManager& _rhs) = delete;
#pragma endregion
};