/*****************************************************************************
 * Header: re_timer
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _RETIMER_H
#define	_RETIMER_H

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif


//--------------------------------------------------------
//--------------------------------------------------------
class reTimer
{
public:
    reTimer();
    void start();
    float getElapsed();
    float peekElapsed();
	float getFPS();
    
protected:
	long m_frameCount;
#ifdef _WIN32
    double m_freqInv;
    __int64 m_start;
    __int64 m_now;
#else
    timeval m_start;
    timeval m_now;
#endif
};

#endif
