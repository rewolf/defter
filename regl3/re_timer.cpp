/*****************************************************************************
 * re_timer: Used to provide timing functions
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/
#include "regl3.h"

//--------------------------------------------------------
reTimer::reTimer()
{
#ifdef _WIN32
    __int64 tempFreq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tempFreq);
    m_freqInv = 1.0f/double(tempFreq);
#endif
	m_frameCount = 0L;
}

//--------------------------------------------------------
void reTimer::start()
{
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
#else
    gettimeofday(&m_start, 0);
#endif
    m_now = m_start;
}

//--------------------------------------------------------
float reTimer::getElapsed()
{
	m_frameCount++;
#ifdef _WIN32
    __int64 temp, elapsed;
    QueryPerformanceCounter((LARGE_INTEGER*)&temp);
    elapsed = temp - m_now;
    m_now += elapsed;
    return ((float)(elapsed * m_freqInv));
#else
    timeval temp;
    gettimeofday(&temp, 0);
    float elapsed = temp.tv_sec - m_now.tv_sec + (temp.tv_usec - m_now.tv_usec) * 1.0e-6;
    m_now = temp;
    return elapsed;
#endif
}

//--------------------------------------------------------
float reTimer::peekElapsed()
{
#ifdef _WIN32
    __int64 temp, elapsed;
    QueryPerformanceCounter((LARGE_INTEGER*)&temp);
    elapsed = temp - m_now;
    return ((float)(elapsed * m_freqInv));
#else
    timeval temp;
    gettimeofday(&temp, 0);
    float elapsed = temp.tv_sec - m_now.tv_sec + (temp.tv_usec - m_now.tv_usec) * 1.0e-6;
    return elapsed;
#endif
}

//--------------------------------------------------------
float reTimer::getFPS()
{
#ifdef _WIN32
	return ((float)(m_frameCount/((m_now - m_start)*m_freqInv)));
#else
        return float(m_frameCount)/float(m_now.tv_sec-m_start.tv_sec +  (m_now.tv_usec - m_start.tv_usec) * 1.0e-6);
#endif        
}
