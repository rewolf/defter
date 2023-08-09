/*****************************************************************************
 * re_input: Class to handle user input to the program
 *
 * Copyright © 2010
 * Authors: Andrew Flower & Justin Crause
 * Emails:	andrew.flower@gmail.com & juzzwuzz@gmail.com
 *****************************************************************************/

#include "regl3.h"

Input::Input()
{
	memset(m_keydown,0, sizeof(m_keydown));
	memset(m_waskeydown,0, sizeof(m_waskeydown));
	memset(m_buttondown,0, sizeof(m_buttondown));
	memset(m_wasbuttondown,0, sizeof(m_wasbuttondown));
	memset(&m_mouse_delta, 0, sizeof(m_mouse_delta));
	m_wheelTicks = 0;
}

void Input::PressKey(SDL_Keycode key)
{
	if (key & (1<<30))
		key^=(1<<30);
	if (key>384) return;
	m_keydown[key] = true;
	m_waskeydown[key] = true;
}

void Input::ReleaseKey(SDL_Keycode key)
{
	if (key & (1<<30))
		key^=(1<<30);
	if (key>384) return;
	m_keydown[key] = false;
}

void Input::PressButton(Uint8 but)
{
	m_buttondown[but] = true;
	m_wasbuttondown[but]=true;
}

void Input::ReleaseButton(Uint8 but)
{
	m_buttondown[but] = false;
}

void Input::WheelUp()
{
	m_wheelTicks++;
}

void Input::WheelDown()
{
	m_wheelTicks--;
}

void Input::MoveMouse(SDL_MouseMotionEvent evt)
{
	m_mouse_pos.x = (float)evt.x;
	m_mouse_pos.y = (float)evt.y;
	m_mouse_delta.x += evt.xrel;
	m_mouse_delta.y += evt.yrel;
}

bool Input::WasKeyPressed(SDL_Keycode key)
{
	if (key & (1<<30))
		key^=(1<<30);
	bool ret = m_waskeydown[key];
	m_waskeydown[key] = false;
	return ret;
}

bool Input::IsKeyPressed(SDL_Keycode key)
{
	if (key & (1<<30))
		key^=(1<<30);
	return m_keydown[key];
}

bool Input::WasButtonPressed(Uint8 but)
{
	bool ret = m_wasbuttondown[but];
	m_wasbuttondown[but] = false;
	return ret;
}

bool Input::IsButtonPressed(Uint8 but)
{
	return m_buttondown[but];
}

int Input::GetWheelTicks()
{
	int temp = m_wheelTicks;
	m_wheelTicks=0;
	return temp;
}

MousePos Input::GetMousePos()
{
	return m_mouse_pos;
}

MouseDelta Input::GetMouseDelta()
{
	MouseDelta temp = m_mouse_delta;
	m_mouse_delta.x = 0;
	m_mouse_delta.y = 0;
	return temp;
}
