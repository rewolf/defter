/*****************************************************************************
 * Header: re_input
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#ifndef _REINPUT_H_
#define _REINPUT_H_

/******************************************************************************
 * Structs for the Input class
 ******************************************************************************/
struct MousePos
{
	float x;
	float y;
};
struct MouseDelta
{
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
	void ClearStates();

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
