#include "stdafx.h"
#include "GameTimer.h"

GameTimer::GameTimer() {
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secondsPerCount = 1.0 / (double)countsPerSec;
}

void GameTimer::Reset() {
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_isStop   = false;
}

void GameTimer::Start() {
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_isStop) {
		m_pausedTime += (startTime - m_stopTime);

		m_prevTime = startTime;
		m_stopTime = 0;
		m_isStop   = false;
	}
}

void GameTimer::Stop() {
	if (!m_isStop) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime;
		m_isStop   = true;
	}
}

void GameTimer::Tick() {
	if (m_isStop) {
		m_deltaTime = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;

	m_deltaTime = (m_currTime - m_prevTime)*m_secondsPerCount;

	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0) {
		m_deltaTime = 0.0;
	}
}

float GameTimer::GetTotalTime() const {
	if (m_isStop) {
		return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
	else {
		return (float)(((m_currTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
	}
}

float GameTimer::GetDeltaTime() const { return (float)m_deltaTime; }