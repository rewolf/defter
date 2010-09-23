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

#ifndef _REINPUT_H_
#define _REINPUT_H_

/******************************************************************************
 * Structs for the Input class
 ******************************************************************************/
struct MousePos{
	float x;
	float y;
};
struct MouseDelta{
	float x;
	float y;
};

/******************************************************************************
 * Input - an input-handler that the reGL3App class uses in order
 * to keep track of keyboard and mouse state.
 ******************************************************************************/
class Input
{
public:
	Input(void);
	~Input(void){}

	void PressKey(SDLKey key);
	void ReleaseKey(SDLKey key);
	void PressButton(Uint8 but);
	void ReleaseButton(Uint8 but);
	void WheelUp();
	void WheelDown();
	void MoveMouse(SDL_MouseMotionEvent evt);

	bool IsKeyPressed(SDLKey key);
	bool WasKeyPressed(SDLKey key);
	bool IsButtonPressed(Uint8 but);
	bool WasButtonPressed(Uint8 but);
	int	 GetWheelTicks();
	MousePos GetMousePos();
	MouseDelta GetMouseDelta();

protected:
	bool		m_keydown[384];
	bool		m_waskeydown[384];
	bool		m_buttondown[256];
	bool		m_wasbuttondown[256];
	MousePos	m_mouse_pos;
	MouseDelta	m_mouse_delta;
	int			m_wheelTicks;
};

#endif

