#include "stdafx.h"
#include "GameTimer.h"

GameTimer::GameTimer()
{
	__int64 countsPerSec;
	// 초당 진동수를 구한다
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	// 진동수당 시간(초)를 구한다
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_isStop   = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// 정지 -> 시작인 경우
	if (m_isStop) {
		// 멈춰있던 시간 += 시작 요청 시점 - 정지 요청 시점
		m_pausedTime += (startTime - m_stopTime);

		m_prevTime = startTime;
		m_stopTime = 0;
		m_isStop   = false;
	}
}

void GameTimer::Stop()
{
	if (!m_isStop) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime;
		m_isStop   = true;
	}
}

void GameTimer::Tick()
{
	if (m_isStop) {
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	// 시간 간격 = 현재 시간 - 이전 시간
	m_deltaTime = (m_currTime - m_prevTime)*m_secondsPerCount;

	m_prevTime = m_currTime;

	// 오류로 인해 0보다 작은 값이 되면 다시 0으로 맞춘다
	if (m_deltaTime < 0.0) {
		m_deltaTime = 0.0;
	}
}

float GameTimer::GetTotalTime() const
{
	// 총 경과 시간
	// 정지O : (정지를 요청한 시간 - 멈춰있었던 시간) - 시작 시간
	if (m_isStop) {
		return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
	// 정지X : (현재 시간 - 멈춰있었던 시간) - 시작 시간
	else {
		return (float)(((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
}

float GameTimer::GetDeltaTime() const
{
	return (float)m_deltaTime;
}