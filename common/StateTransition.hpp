#pragma once

#include "pch.h"

namespace Common
{
	// 상태 전이 조건 클래스
	// 상태 전이 조건을 만족하는지 확인하는 함수를 제공
	template <class State>
	class StateTransitionBase
	{
	public:
		StateTransitionBase(State toState) : m_toState(toState)
		{
		}

		virtual bool IsTriggered(float time) const = 0;
		State GetToState() const { return m_toState; }

	private:
		State m_toState;
	};

	// Bool 상태 전이 매개변수에 대한 상태 전이 조건 클래스
	// condition이 trigger와 같은지 확인하여 상태 전이 여부를 결정
	template <class State>
	class BoolStateTransition : public StateTransitionBase<State>
	{
	public:
		BoolStateTransition(State toState, bool* condition, bool trigger) : StateTransitionBase<State>(toState), m_condition(condition), m_trigger(trigger)
		{
		}

		bool IsTriggered(float time) const override
		{
			return *m_condition == m_trigger;
		}

	private:
		bool* m_condition;
		bool m_trigger;
	};

	// Float 상태 전이 매개변수에 대한 상태 전이 조건 클래스
	// condition과 thresold 사이의 관계를 Comp로 확인하여 상태 전이 여부를 결정
	template <class State, class Comp>
	class FloatStateTransition : public StateTransitionBase<State>
	{
	public:
		FloatStateTransition(State toState, float* condition, float thresold) : StateTransitionBase<State>(toState), m_condition(condition), m_thresold(thresold)
		{
		}

		bool IsTriggered(float time) const override
		{
			return Comp()(*m_condition, m_thresold);
		}

	private:
		float* m_condition;
		float m_thresold;
	};

	// Int 상태 전이 매개변수에 대한 상태 전이 조건 클래스
	// condition과 thresold 사이의 관계를 Comp로 확인하여 상태 전이 여부를 결정
	template <class State, class Comp>
	class IntStateTransition : public StateTransitionBase<State>
	{
	public:
		IntStateTransition(State toState, int* condition, int thresold) : StateTransitionBase<State>(toState), m_condition(condition), m_thresold(thresold)
		{
		}

		bool IsTriggered(float time) const override
		{
			return Comp()(*m_condition, m_thresold);
		}

	private:
		int* m_condition;
		int m_thresold;
	};

	// 상태의 지속시간에 대한 상태 전이 조건 클래스
	// duration이 지나면 상태 전이를 수행
	template <class State>
	class TimerStateTransition : public StateTransitionBase<State>
	{
	public:
		TimerStateTransition(State toState, float duration) : StateTransitionBase<State>(toState), m_duration(duration)
		{
		}

		bool IsTriggered(float time) const override
		{
			return time >= m_duration;
		}

	private:
		float m_duration;
	};
}