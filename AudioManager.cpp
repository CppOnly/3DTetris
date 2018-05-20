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

AudioManager::AudioManager()
{
	assert(!m_isCreate);
	m_isCreate = true;

	if (!InitXAudio2()) {
		assert(false);
	}
}

AudioManager::~AudioManager()
{
	pMasterVoice->DestroyVoice();

	for (auto &a : pAudioes) {
		a->DestroyVoice();
	}
}



bool AudioManager::InitXAudio2()
{
	ThrowIfFailed(XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
	ThrowIfFailed(m_xAudio2->CreateMasteringVoice(&pMasterVoice));

	if (!BuildMainAudio()) {
		return false;
	}
	if (!BuildWinAudio()) {
		return false;
	}
	if (!BuildLoseAudio()) {
		return false;
	}
	if (!BuildDropAudio()) {
		return false;
	}

	return true;
}


// Main Audio는 그냥 Loop가 적용된 파일을 Start, Stop만 하면 된다.
void AudioManager::PlayMainAudio()
{
	pAudioes[MAIN]->Start(0);
}

void AudioManager::StopMainAudio()
{
	pAudioes[MAIN]->Stop(0);
}

// Win Audio는 시작할 때 Buffer를 넣어 처음부터 재생하도록 한다.
void AudioManager::PlayWinAudio()
{
	ThrowIfFailed(pAudioes[WIN]->SubmitSourceBuffer(&buffer[WIN]));
	pAudioes[WIN]->Start(0);
}

// 종료할 때는 Buffer를 싹 비워서 다음 재생 때 처음부터 재생되도록 한다.
void AudioManager::StopWinAudio()
{
	pAudioes[WIN]->Stop();
	pAudioes[WIN]->FlushSourceBuffers();
}

void AudioManager::PlayLoseAudio()
{
	ThrowIfFailed(pAudioes[LOSE]->SubmitSourceBuffer(&buffer[LOSE]));
	pAudioes[LOSE]->Start(0);
}

void AudioManager::StopLoseAudio()
{
	pAudioes[LOSE]->Stop();
	pAudioes[LOSE]->FlushSourceBuffers();
}

// Drop과 Clear는 중복 재생을 방지하기 위해 Audio Source에 Buffer가 있는지 확인하고 없으면 그 때 넘겨주어 재생되도록 한다.
void AudioManager::PlayDropAudio()
{
	XAUDIO2_VOICE_STATE avs;
	pAudioes[DROP]->GetState(&avs);
	if (avs.BuffersQueued == 0) {
		ThrowIfFailed(pAudioes[DROP]->SubmitSourceBuffer(&buffer[DROP]));
	}
	pAudioes[DROP]->Start(0);
}



bool AudioManager::BuildMainAudio()
{
	// Loop가 적용된 Audio Buffer(Main)을 만든다.
	if (!LoadWAVFromFile(_TEXT("media\\Main.wav"), wfx, buffer[MAIN], true)) {
		return false;
	}
	// Audio Source를 만들어주고
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[MAIN], (WAVEFORMATEX*)&wfx));
	pAudioes[MAIN]->SetVolume(0.5f);
	// Buffer를 Build 단계에서 곧장 Audio Source로 넘겨준다. 처음부터 다시 시작할 필요가 없기 때문이다.
	ThrowIfFailed(pAudioes[MAIN]->SubmitSourceBuffer(&buffer[MAIN]));

	return true;
}

bool AudioManager::BuildWinAudio()
{
	// Loop가 적용된 Audio Buffer(Win)을 만든다.
	if (!LoadWAVFromFile(_TEXT("media\\Win.wav"), wfx, buffer[WIN], true)) {
		return false;
	}
	// Audio Source를 만들어주고
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[WIN], (WAVEFORMATEX*)&wfx));
	pAudioes[WIN]->SetVolume(0.5f);

	// 그냥 끝낸다. 이유는 다시 시작 할때 처음부터 재생해야하기 때문에 Start에서 Buffer를 넘겨줘야 하기 때문이다.
	return true;
}

bool AudioManager::BuildLoseAudio()
{
	// Loop가 적용된 Audio Buffer(Lose)을 만든다.
	if (!LoadWAVFromFile(_TEXT("media\\Lose.wav"), wfx, buffer[LOSE], true)) {
		return false;
	}
	// Audio Source를 만들어주고
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[LOSE], (WAVEFORMATEX*)&wfx));
	pAudioes[LOSE]->SetVolume(2.0f);

	// 그냥 끝낸다. 이유는 다시 시작 할때 처음부터 재생해야하기 때문에 Start에서 Buffer를 넘겨줘야 하기 때문이다.
	return true;
}

bool AudioManager::BuildDropAudio()
{
	// Loop가 없는 Audio Buffer(Drop)을 만든다.
	if (!LoadWAVFromFile(_TEXT("media\\Drop.wav"), wfx, buffer[DROP])) {
		return false;
	}
	// Audio Source를 만들어주고
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[DROP], (WAVEFORMATEX*)&wfx));
	pAudioes[DROP]->SetVolume(200.0f);

	// 그냥 끝낸다. 이유는 다시 시작 할때 처음부터 재생해야하기 때문에 Start에서 Buffer를 넘겨줘야 하기 때문이다.
	return true;
}



bool AudioManager::LoadWAVFromFile(TCHAR* _strFileName, WAVEFORMATEXTENSIBLE &_wfx, XAUDIO2_BUFFER &_buffer, bool _isLoop)
{
	HANDLE hFile = CreateFile(_strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return false;
	}

	DWORD chunkSize;
	DWORD chunkOffset;

	// 'RIFF' Data
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

	// 'FMT ' Data
	if (!FindChunk(hFile, FMT, chunkSize, chunkOffset)) {
		return false;
	}
	if (!ReadChunkData(hFile, &_wfx, chunkSize, chunkOffset)) {
		return false;
	}

	// 'Data' Data
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

bool AudioManager::FindChunk(HANDLE _hFile, DWORD _chunkID, DWORD &_chunkSize, DWORD &_chunkOffset)
{
	// FilePointer의 유효성을 검사한다.
	if (SetFilePointer(_hFile, 0, NULL, FILE_BEGIN == INVALID_SET_FILE_POINTER)) {
		return false;
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD offset = 0;
	DWORD temp;

	bool i = true;
	while (i == true)
	{
		DWORD dwRead;
		// WAV를 읽어오되 4Byte만 읽어오고 이를 chunkType에 저장한다.
		if (ReadFile(_hFile, &chunkType, sizeof(DWORD), &dwRead, NULL) == 0) {
			i = false;
		}

		// 다음 4Byte를 읽어오고 이를 chunkDataSize에 저장한다.
		if (ReadFile(_hFile, &chunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0) {
			i = false;
		}

		switch (chunkType)
		{
			// 현재 분석중인 Header가 RIFF Header인 경우
		case RIFF:
			// 예외적으로 RIFF Header의 Size영역에는 RIFF의 Size값이 아니라 File의 Size값이 들어있다.
			// 이를 이용하여 현재 Load된 File의 간단한 유효성 검사를 할 수 있다.
			if (chunkDataSize <= 0) {
				return false;
			}
			// RIFF의 Size값은 'WAV' ASCII만 담으면되는 4Byte이므로 수정해준다.
			chunkDataSize = 4;

			// 다음 4Byte를 읽어온다. (Pointer 옮기기용)
			if (ReadFile(_hFile, &temp, sizeof(DWORD), &dwRead, NULL) == 0) {
				return false;
			}
			break;

			// 그 외의 Header(FMT, DATA)인 경우
		default:
			// FilePointer를 옮겨준다.
			if (INVALID_SET_FILE_POINTER == SetFilePointer(_hFile, chunkDataSize, NULL, FILE_CURRENT)) {
				return false;
			}
		}

		// Offset을 8Byte(ID + Size 영역)만큼 늘려준다.
		offset += sizeof(DWORD) * 2;

		// Read 과정을 통해 찾은 HeaderID와 찾고자 하는 HeaderID를 대조하여 일치한다면 그대로 필요한 값들을 반환한다.
		if (chunkType == _chunkID) {
			_chunkSize = chunkDataSize;
			_chunkOffset = offset;
			return true;
		}

		// 다음 Header로 Offset을 조정한다.
		offset += chunkDataSize;
	}
	return true;
}

bool AudioManager::ReadChunkData(HANDLE _hFile, void* _buffer, DWORD _bufferSize, DWORD _bufferOffset)
{
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