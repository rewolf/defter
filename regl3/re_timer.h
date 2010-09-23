/*
    Copyright (C) 2010 Andrew Flower <andrew.flower@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _RETIMER_H
#define	_RETIMER_H

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif


//--------------------------------------------------------
//--------------------------------------------------------
class reTimer{
public:
    reTimer();
    void start();
    float getElapsed();
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


#endif	/* _reTimer_H */

