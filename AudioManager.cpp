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


// Main Audio�� �׳� Loop�� ����� ������ Start, Stop�� �ϸ� �ȴ�.
void AudioManager::PlayMainAudio()
{
	pAudioes[MAIN]->Start(0);
}

void AudioManager::StopMainAudio()
{
	pAudioes[MAIN]->Stop(0);
}

// Win Audio�� ������ �� Buffer�� �־� ó������ ����ϵ��� �Ѵ�.
void AudioManager::PlayWinAudio()
{
	ThrowIfFailed(pAudioes[WIN]->SubmitSourceBuffer(&buffer[WIN]));
	pAudioes[WIN]->Start(0);
}

// ������ ���� Buffer�� �� ����� ���� ��� �� ó������ ����ǵ��� �Ѵ�.
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

// Drop�� Clear�� �ߺ� ����� �����ϱ� ���� Audio Source�� Buffer�� �ִ��� Ȯ���ϰ� ������ �� �� �Ѱ��־� ����ǵ��� �Ѵ�.
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
	// Loop�� ����� Audio Buffer(Main)�� �����.
	if (!LoadWAVFromFile(_TEXT("media\\Main.wav"), wfx, buffer[MAIN], true)) {
		return false;
	}
	// Audio Source�� ������ְ�
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[MAIN], (WAVEFORMATEX*)&wfx));
	pAudioes[MAIN]->SetVolume(0.5f);
	// Buffer�� Build �ܰ迡�� ���� Audio Source�� �Ѱ��ش�. ó������ �ٽ� ������ �ʿ䰡 ���� �����̴�.
	ThrowIfFailed(pAudioes[MAIN]->SubmitSourceBuffer(&buffer[MAIN]));

	return true;
}

bool AudioManager::BuildWinAudio()
{
	// Loop�� ����� Audio Buffer(Win)�� �����.
	if (!LoadWAVFromFile(_TEXT("media\\Win.wav"), wfx, buffer[WIN], true)) {
		return false;
	}
	// Audio Source�� ������ְ�
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[WIN], (WAVEFORMATEX*)&wfx));
	pAudioes[WIN]->SetVolume(0.5f);

	// �׳� ������. ������ �ٽ� ���� �Ҷ� ó������ ����ؾ��ϱ� ������ Start���� Buffer�� �Ѱ���� �ϱ� �����̴�.
	return true;
}

bool AudioManager::BuildLoseAudio()
{
	// Loop�� ����� Audio Buffer(Lose)�� �����.
	if (!LoadWAVFromFile(_TEXT("media\\Lose.wav"), wfx, buffer[LOSE], true)) {
		return false;
	}
	// Audio Source�� ������ְ�
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[LOSE], (WAVEFORMATEX*)&wfx));
	pAudioes[LOSE]->SetVolume(2.0f);

	// �׳� ������. ������ �ٽ� ���� �Ҷ� ó������ ����ؾ��ϱ� ������ Start���� Buffer�� �Ѱ���� �ϱ� �����̴�.
	return true;
}

bool AudioManager::BuildDropAudio()
{
	// Loop�� ���� Audio Buffer(Drop)�� �����.
	if (!LoadWAVFromFile(_TEXT("media\\Drop.wav"), wfx, buffer[DROP])) {
		return false;
	}
	// Audio Source�� ������ְ�
	ThrowIfFailed(m_xAudio2->CreateSourceVoice(&pAudioes[DROP], (WAVEFORMATEX*)&wfx));
	pAudioes[DROP]->SetVolume(200.0f);

	// �׳� ������. ������ �ٽ� ���� �Ҷ� ó������ ����ؾ��ϱ� ������ Start���� Buffer�� �Ѱ���� �ϱ� �����̴�.
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
	// FilePointer�� ��ȿ���� �˻��Ѵ�.
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
		// WAV�� �о���� 4Byte�� �о���� �̸� chunkType�� �����Ѵ�.
		if (ReadFile(_hFile, &chunkType, sizeof(DWORD), &dwRead, NULL) == 0) {
			i = false;
		}

		// ���� 4Byte�� �о���� �̸� chunkDataSize�� �����Ѵ�.
		if (ReadFile(_hFile, &chunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0) {
			i = false;
		}

		switch (chunkType)
		{
			// ���� �м����� Header�� RIFF Header�� ���
		case RIFF:
			// ���������� RIFF Header�� Size�������� RIFF�� Size���� �ƴ϶� File�� Size���� ����ִ�.
			// �̸� �̿��Ͽ� ���� Load�� File�� ������ ��ȿ�� �˻縦 �� �� �ִ�.
			if (chunkDataSize <= 0) {
				return false;
			}
			// RIFF�� Size���� 'WAV' ASCII�� ������Ǵ� 4Byte�̹Ƿ� �������ش�.
			chunkDataSize = 4;

			// ���� 4Byte�� �о�´�. (Pointer �ű���)
			if (ReadFile(_hFile, &temp, sizeof(DWORD), &dwRead, NULL) == 0) {
				return false;
			}
			break;

			// �� ���� Header(FMT, DATA)�� ���
		default:
			// FilePointer�� �Ű��ش�.
			if (INVALID_SET_FILE_POINTER == SetFilePointer(_hFile, chunkDataSize, NULL, FILE_CURRENT)) {
				return false;
			}
		}

		// Offset�� 8Byte(ID + Size ����)��ŭ �÷��ش�.
		offset += sizeof(DWORD) * 2;

		// Read ������ ���� ã�� HeaderID�� ã���� �ϴ� HeaderID�� �����Ͽ� ��ġ�Ѵٸ� �״�� �ʿ��� ������ ��ȯ�Ѵ�.
		if (chunkType == _chunkID) {
			_chunkSize = chunkDataSize;
			_chunkOffset = offset;
			return true;
		}

		// ���� Header�� Offset�� �����Ѵ�.
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