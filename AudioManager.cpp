#include "stdafx.h"
#include "AudioManager.h"
#include "D3D12Helper.h"

#pragma comment(lib, "XAudio2_8" )

#define RIFF 'FFIR'
#define DATA 'atad'
#define FMT  ' tmf'
#define WAVE 'EVAW'
#define XWMA 'AMWX'
#define DPDS 'sdpd'

AudioManager::AudioManager() {
	assert(!m_isCreate);
	m_isCreate = true;

	if (!InitXAudio2()) {
		assert(false);
	}
}

AudioManager::~AudioManager() {
	pMasterVoice->DestroyVoice();

	for (auto &a : pAudioes) {
		a->DestroyVoice();
	}
}

bool AudioManager::InitXAudio2() {
	ThrowIfFailed(XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
	ThrowIfFailed(m_xAudio2->CreateMasteringVoice(&pMasterVoice));

	if (!BuildMainAudio()) { return false; }
	if (!BuildWinAudio())  { return false; }
	if (!BuildLoseAudio()) { return false; }
	if (!BuildDropAudio()) { return false; }

	return true;
}

void AudioManager::PlayMainAudio() {
	pAudioes[MAIN]->Start(0);
}

void AudioManager::StopMainAudio() {
	pAudioes[MAIN]->Stop(0);
}

void AudioManager::PlayWinAudio() {
	ThrowIfFailed(pAudioes[WIN]->SubmitSourceBuffer(&buffer[WIN]));
	pAudioes[WIN]->Start(0);
}

void AudioManager::StopWinAudio() {
	pAudioes[WIN]->Stop();
	pAudioes[WIN]->FlushSourceBuffers();
}

void AudioManager::PlayLoseAudio() {
	ThrowIfFailed(pAudioes[LOSE]->SubmitSourceBuffer(&buffer[LOSE]));
	pAudioes[LOSE]->Start(0);
}

void AudioManager::StopLoseAudio() {
	pAudioes[LOSE]->Stop();
	pAudioes[LOSE]->FlushSourceBuffers();
}

void AudioManager::PlayDropAudio() {
	XAUDIO2_VOICE_STATE avs;
	pAudioes[DROP]->GetState(&avs);
	if (avs.BuffersQueued == 0) {
		ThrowIfFailed(pAudioes[DROP]->SubmitSourceBuffer(&buffer[DROP]));
	}
	pAudioes[DROP]->Start(0);
}

bool AudioManager::BuildMainAudio() {
	if (!LoadWAVFromFile(_TEXT("media\\Main.wav"), wfx, buffer[MAIN], true)) {
		return false;
	}
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[MAIN], (WAVEFORMATEX*)&wfx));
	pAudioes[MAIN]->SetVolume(0.5f);
	ThrowIfFailed(pAudioes[MAIN]->SubmitSourceBuffer(&buffer[MAIN]));

	return true;
}

bool AudioManager::BuildWinAudio() {
	if (!LoadWAVFromFile(_TEXT("media\\Win.wav"), wfx, buffer[WIN], true)) {
		return false;
	}
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[WIN], (WAVEFORMATEX*)&wfx));
	pAudioes[WIN]->SetVolume(0.5f);

	return true;
}

bool AudioManager::BuildLoseAudio() {
	if (!LoadWAVFromFile(_TEXT("media\\Lose.wav"), wfx, buffer[LOSE], true)) {
		return false;
	}
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[LOSE], (WAVEFORMATEX*)&wfx));
	pAudioes[LOSE]->SetVolume(2.0f);

	return true;
}

bool AudioManager::BuildDropAudio() {
	if (!LoadWAVFromFile(_TEXT("media\\Drop.wav"), wfx, buffer[DROP])) {
		return false;
	}
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[DROP], (WAVEFORMATEX*)&wfx));
	pAudioes[DROP]->SetVolume(200.0f);

	return true;
}

bool AudioManager::LoadWAVFromFile(TCHAR* _strFileName, WAVEFORMATEXTENSIBLE &_wfx, XAUDIO2_BUFFER &_buffer, bool _isLoop) {
	HANDLE hFile = CreateFile(_strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return false;
	}

	DWORD chunkSize;
	DWORD chunkOffset;

	DWORD filetype;
	if (!FindChunk(hFile, RIFF, chunkSize, chunkOffset)) {
		return false;
	}
	if (!ReadChunkData(hFile, &filetype, chunkSize, chunkOffset)) {
		return false;
	}
	if (filetype != WAVE) {
		return false;
	}

	if (!FindChunk(hFile, FMT, chunkSize, chunkOffset)) {
		return false;
	}
	if (!ReadChunkData(hFile, &_wfx, chunkSize, chunkOffset)) {
		return false;
	}

	if (!FindChunk(hFile, DATA, chunkSize, chunkOffset)) {
		return false;
	}
	BYTE* pDataBuffer = new BYTE[chunkSize];
	if (!ReadChunkData(hFile, pDataBuffer, chunkSize, chunkOffset)) {
		return false;
	}

	_buffer.AudioBytes = chunkSize;
	_buffer.pAudioData = pDataBuffer;
	_buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (_isLoop == true) {
		_buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	return true;
}

bool AudioManager::FindChunk(HANDLE _hFile, DWORD _chunkID, DWORD &_chunkSize, DWORD &_chunkOffset) {
	if (SetFilePointer(_hFile, 0, NULL, FILE_BEGIN == INVALID_SET_FILE_POINTER)) {
		return false;
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD offset = 0;
	DWORD temp;
	bool check = true;
	while (check == true) {
		DWORD dwRead;
		if (ReadFile(_hFile, &chunkType, sizeof(DWORD), &dwRead, NULL) == 0) {
			check = false;
		}

		if (ReadFile(_hFile, &chunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0) {
			check = false;
		}

		switch (chunkType) {
		case RIFF:
			if (chunkDataSize <= 0) {
				return false;
			}
			chunkDataSize = 4;

			if (ReadFile(_hFile, &temp, sizeof(DWORD), &dwRead, NULL) == 0) {
				return false;
			}
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(_hFile, chunkDataSize, NULL, FILE_CURRENT)) {
				return false;
			}
		}

		offset += sizeof(DWORD) * 2;

		if (chunkType == _chunkID) {
			_chunkSize = chunkDataSize;
			_chunkOffset = offset;
			return true;
		}

		offset += chunkDataSize;
	}
	return true;
}

bool AudioManager::ReadChunkData(HANDLE _hFile, void* _buffer, DWORD _bufferSize, DWORD _bufferOffset) {
	if (SetFilePointer(_hFile, _bufferOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return false;
	}

	DWORD temp;
	if (ReadFile(_hFile, _buffer, _bufferSize, &temp, NULL) == 0) {
		return false;
	}
	return true;
}

bool AudioManager::m_isCreate = false;