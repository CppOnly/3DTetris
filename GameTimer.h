#pragma once
#include "stdafx.h"

class GameTimer
{
public:
	GameTimer();

	void Reset();
	void Start();
	void Stop();
	void Tick();

	float GetDeltaTime() const;
	float GetTotalTime() const;

private:
	double m_secondsPerCount = 0.0;
	double m_deltaTime = -1.0;

	__int64 m_baseTime = 0;
	__int64 m_pausedTime = 0;
	__int64 m_stopTime = 0;
	__int64 m_prevTime = 0;
	__int64 m_currTime = 0;

	bool m_isStop = false;
};