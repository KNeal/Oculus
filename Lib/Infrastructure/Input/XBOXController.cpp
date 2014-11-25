#include "stdafx.h"
#include "XBOXController.h"
#include "XBOXControllerButtonEvent.h"
#include "XBOXControllerUpdateEvent.h"
#include <math.h>
#include "General\EventBus.hpp"

namespace Infrastructure
{

//-----------------------------------------------------------------------------------
// Life Cycle
//-----------------------------------------------------------------------------------

XBOXController::XBOXController(int playerNumber)
: m_deadzoneLeftThumb(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
, m_deadzoneRightThumb(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
, m_triggerThreshold(XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
, m_isConnected(false)
{
    // Set the Controller Number
    m_controllerNum = playerNumber - 1;

	// Initialize the States
	ZeroMemory(&m_previousState, sizeof(XINPUT_STATE));
    ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
}

//-----------------------------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------------------------

void XBOXController::Update()
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	DWORD Result = XInputGetState(m_controllerNum, &state);
	if(Result == ERROR_SUCCESS)
    {
		m_isConnected = true;

		// Update the states only when they have changed
		if(m_currentState.dwPacketNumber != state.dwPacketNumber)	
		{
			CopyMemory(&m_previousState, &m_currentState, sizeof(XINPUT_STATE));
			CopyMemory(&m_currentState, &state, sizeof(XINPUT_STATE));
		}

		BroadcastEvents();	
    }
    else
    {		
	   m_isConnected = false;

	   // Clear the states.
       ZeroMemory(&m_previousState, sizeof(XINPUT_STATE));
	   ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
    }
}

void XBOXController::Vibrate(int leftVal, int rightVal)
{
	if(!IsConnected())
	{
		return;
	}

    // Create a Vibraton State
    XINPUT_VIBRATION vibration;

    // Zeroise the Vibration
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

    // Set the Vibration Values
    vibration.wLeftMotorSpeed = leftVal;
    vibration.wRightMotorSpeed = rightVal;

    // Vibrate the controller
    XInputSetState(m_controllerNum, &vibration);
}

bool XBOXController::GetLeftThumbStickPosition(Vec2 &outValue)
{
	if(!IsConnected())
	{
		outValue.x = 0.0f;
		outValue.y = 0.0f;
		return false;
	}

	XINPUT_STATE state = GetCurrentState();
	NormalizeThumbStickPosition( state.Gamepad.sThumbLX,  state.Gamepad.sThumbLY, m_deadzoneLeftThumb, outValue);
	return true;
}

bool XBOXController::GetRightThumbStickPosition(Vec2 &outValue)
{
	if(!IsConnected())
	{
		outValue.x = 0.0f;
		outValue.y = 0.0f;;
		return false;
	}

	XINPUT_STATE state = GetCurrentState();
    NormalizeThumbStickPosition( state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, m_deadzoneRightThumb, outValue);
	return true;
}

float XBOXController::GetLeftTriggerPosition()
{
	if(!IsConnected())
	{
		return 0;
	}

	XINPUT_STATE state = GetCurrentState();
	return NormalizeTiggerPosition(state.Gamepad.bLeftTrigger, m_triggerThreshold);
}
float XBOXController::GetRightTriggerPosition()
{
	if(!IsConnected())
	{
		return 0;
	}

	XINPUT_STATE state = GetCurrentState();
	return NormalizeTiggerPosition(state.Gamepad.bRightTrigger, m_triggerThreshold);
}

//-----------------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------------

void XBOXController::NormalizeThumbStickPosition(int x, int y, int deadZone, Vec2 &outValue)
{
	//determine how far the controller is pushed
	float magnitude = sqrt(float(x*x) + float(y*y));

	//check if the controller is outside a circular dead zone
	if (magnitude > deadZone)
	{
	  //determine the direction the controller is pushed
	  outValue.x = min(max(float(x)/magnitude, -1.0f), 1.0f);
	  outValue.y = min(max(float(y)/magnitude, -1.0f), 1.0f);
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		outValue.x = 0.0f;
		outValue.y = 0.0f;
	}
}

float XBOXController::NormalizeTiggerPosition(int value, int deadZone)
{
	if (value > deadZone)
	{
	  //clip the magnitude at its expected maximum value
	  if (value > 32767) value = 32767;
  
	  //adjust magnitude relative to the end of the dead zone
	  value -= deadZone;

	  //optionally normalize the magnitude with respect to its expected range
	  //giving a magnitude value of 0.0 to 1.0
	  float normalizedValue = float(value) / (32767 - deadZone);

	  return normalizedValue;
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		return 0;
	}
}

void XBOXController::BroadcastEvents()
{
	BroadcastButtonEvents(XBOXButtonId::DPAD_UP);
	BroadcastButtonEvents(XBOXButtonId::DPAD_DOWN);
	BroadcastButtonEvents(XBOXButtonId::DPAD_LEFT);
	BroadcastButtonEvents(XBOXButtonId::DPAD_RIGHT);
	BroadcastButtonEvents(XBOXButtonId::START);
	BroadcastButtonEvents(XBOXButtonId::BACK);
	BroadcastButtonEvents(XBOXButtonId::LEFT_THUMB);
	BroadcastButtonEvents(XBOXButtonId::RIGHT_THUMB);
	BroadcastButtonEvents(XBOXButtonId::LEFT_SHOULDER);
	BroadcastButtonEvents(XBOXButtonId::RIGHT_SHOULDER);
	BroadcastButtonEvents(XBOXButtonId::GAMEPAD_A);
	BroadcastButtonEvents(XBOXButtonId::GAMEPAD_B);
	BroadcastButtonEvents(XBOXButtonId::GAMEPAD_X);
	BroadcastButtonEvents(XBOXButtonId::GAMEPAD_Y);

	Vec2 leftThumb, rightThumb;
	GetLeftThumbStickPosition(leftThumb);
	GetRightThumbStickPosition(rightThumb);

	XBOXControllerUpdateEvent e(this, this, leftThumb, rightThumb);
	EventBus::FireEvent(e);
}

void XBOXController::BroadcastButtonEvents(XBOXButtonId button)
{
	int previousSet = m_previousState.Gamepad.wButtons & button;
	int currentSet = m_currentState.Gamepad.wButtons & button;
	if( !previousSet && currentSet )
	{
		XBOXControllerButtonEvent e(this, this, button, ButtonEventType::BUTTON_PRESSED);
		EventBus::FireEvent(e);
	}
	else if( previousSet && !currentSet )
	{
		XBOXControllerButtonEvent e(this, this, button, ButtonEventType::BUTTON_RELEASED);
		EventBus::FireEvent(e);
	}
}

}