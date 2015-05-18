#pragma once

/*!
	@file   Timer.h
	@brief  ŽžŠÔŒv‘ª—p‚ÌClass
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

