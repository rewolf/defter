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

#include "regl3.h"

//--------------------------------------------------------
reTimer::reTimer(){
#ifdef _WIN32
    __int64 tempFreq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tempFreq);
    m_freqInv = 1.0f/double(tempFreq);
#endif
	m_frameCount = 0L;
}

//--------------------------------------------------------
void reTimer::start(){
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
#else
    gettimeofday(&m_start, 0);
#endif
    m_now = m_start;
}

//--------------------------------------------------------
float reTimer::getElapsed(){
	m_frameCount++;
#ifdef _WIN32
    __int64 temp, elapsed;
    QueryPerformanceCounter((LARGE_INTEGER*)&temp);
    elapsed = temp - m_now;
    m_now += elapsed;
    return elapsed * m_freqInv;
#else
    timeval temp;
    gettimeofday(&temp, 0);
    float elapsed = temp.tv_sec - m_now.tv_sec + (temp.tv_usec - m_now.tv_usec) * 1.0e-6;
    m_now = temp;
    return elapsed;
#endif
}

float reTimer::getFPS(){
#ifdef _WIN32
	return m_frameCount/((m_now - m_start)*m_freqInv);
#else
        return float(m_frameCount)/float(m_now.tv_sec-m_start.tv_sec +  (m_now.tv_usec - m_start.tv_usec) * 1.0e-6);
#endif        
}

