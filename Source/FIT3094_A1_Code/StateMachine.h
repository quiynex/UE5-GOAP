#pragma once
/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008 John Downey (jtdowney@purdue.edu)                       */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */
/*----------------------------------------------------------------------------*/

// Modified by Jason Haasz, Josh Olsen, and Nic Pallant for Monash University

// A template/generic programming based approach to a statemachine.

#include <map>

// E - a user defined enum/index for a unique state
// T - the class that owns the state machine.

template<class E, class T>
class StateMachine
{
public:
	typedef void (T::*CallbackOnEnter)();
	typedef void (T::*CallbackOnTick)(float);
	typedef void (T::*CallbackOnExit)();

private:

	T* m_pOwner;
	E m_currentState;

	std::map<E, CallbackOnTick> m_statesOnTick;
	std::map<E, CallbackOnEnter> m_statesOnEnter;
	std::map<E, CallbackOnExit> m_statesOnExit;

public:
	StateMachine(T* pOwner, E emptyState)
	{
		m_currentState = emptyState;
		m_pOwner = pOwner;
	}

	~StateMachine()
	{
	}

	void RegisterState(E state, CallbackOnEnter callbackEntry, CallbackOnTick callbackTick, CallbackOnExit callbackExit)
	{
		m_statesOnEnter[state] = callbackEntry;
		m_statesOnTick[state] = callbackTick;
		m_statesOnExit[state] = callbackExit;
	}

	E GetCurrentState(void)
	{
		return m_currentState;
	}

	void ChangeState(E statenext)
	{
		if (m_pOwner)
		{
			CallbackOnExit callbackExit = m_statesOnExit[m_currentState];
			if (callbackExit)
			{
				// Exit old state
				(m_pOwner->*callbackExit)();
			}
		}

		m_currentState = statenext;

		if (m_pOwner)
		{
			CallbackOnEnter callbackEnter = m_statesOnEnter[m_currentState];
			if (callbackEnter)
			{
				// Enter new state
				(m_pOwner->*callbackEnter)();
			}
		}
	}

	//Update Current State
	void Tick(float fTimeStep)
	{
		if (m_pOwner)
		{
			CallbackOnTick callback = m_statesOnTick[m_currentState];
			if (callback)
			{
				(m_pOwner->*callback)(fTimeStep);
			}
		}
	}

};
