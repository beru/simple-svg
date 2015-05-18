#pragma once

/*!
	@file   Timer.h
	@brief  ���Ԍv���p��Class
*/

class Timer
{
private:
	struct Impl;
	Impl* pImpl_;
	
public:
	Timer();
	~Timer();
	void Start();
	__int64 Elapsed() const;
	double ElapsedSecond() const;
	
};

