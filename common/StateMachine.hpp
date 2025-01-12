#pragma once

#include "pch.h"
#include "StateTransition.hpp"

namespace Common
{
	// 어떤 Enum 클래스를 상태로 사용하며, 상태 전이 조건을 만족할 때 다음 상태로 전이하는 상태 머신 클래스
	// 사용자는 상태 전이 조건을 추가하고, 상태 전이 매개변수를 조작함으로써 상태 전이를 제어할 수 있다.
	// 유니티의 Animator 컴포넌트와 유사한 기능을 제공한다.
	// 중요: State Enum 클래스는 마지막 원소로 'Size'를 가져야 한다.
	template <class State>
	class StateMachine
	{
	public:
		StateMachine(State beginState) : m_currentState(beginState)
		{
		}

	public:
		// 프레임워크에서 갱신 함수를 인계받아 상태 전이를 처리하는 함수
		// 현재 상태에 대한 전이 조건들을 모두 체크하고 하나라도 만족하면 상태를 전이한다.
		// (전이 조건 간 AND 연산을 만족할 때만 전이하는 경우는 추후 추가할 수 있음)
		void Update(float deltaTime)
		{
			m_timeInCurrentState += deltaTime;

			int currentStateIndex = static_cast<int>(m_currentState);
			auto& currentTransitions = m_transitions[currentStateIndex];

			for (const auto& transition : currentTransitions)
			{
				if (transition->IsTriggered(m_timeInCurrentState))
				{
					SetState(transition->GetToState());
					break;
				}
			}
		}

		// 상태 전이 조건을 추가하는 함수
		// 현재 상태 -> 다음 상태를 잇는 단방향 연결 엣지를 추가한다.
		template <class Transition, class... Args>
		void AddTransition(State fromState, Args&&... args)
		{
			size_t fromStateIndex = static_cast<size_t>(fromState);
			m_transitions[fromStateIndex].emplace_back(std::make_unique<Transition>(std::forward<Args>(args)...));
		}

		// 상태 전이 매개변수를 가져오거나 추가하는 함수들
		// 값의 포인터를 가져와 캐싱하여 사용함으로써, 매 프레임마다 키를 찾는 데 발생하는 오버헤드를 줄인다.
		int* GetConditionRefInt(std::string_view key)
		{
			auto& condition = m_conditions[key.data()];
			if (!condition)
			{
				//condition = std::make_unique<int>();
			}
			return reinterpret_cast<int*>(&condition);
		}

		float* GetConditionRefFloat(std::string_view key)
		{
			auto& condition = m_conditions[key.data()];
			if (!condition)
			{
				//condition = std::make_unique<float>();
			}
			return reinterpret_cast<float*>(&condition);
		}

		bool* GetConditionRefBool(std::string_view key)
		{
			auto& condition = m_conditions[key.data()];
			if (!condition)
			{
				//condition = std::make_unique<bool>();
			}
			return reinterpret_cast<bool*>(&condition);
		}

		void SetState(State state)
		{
			State previousState = m_currentState;
			m_currentState = state;
			m_timeInCurrentState = 0.0f;
			for (const auto& callback : m_onStateChangeCallbacks)
			{
				callback(previousState, state);
			}
		}

		// 현재 상태와 진행 시간을 가져오는 함수들
		State GetCurrentState() const { return m_currentState; }
		float GetTimeInCurrentState() const { return m_timeInCurrentState; }

		// 상태 전이 시 콜백 함수를 추가하는 함수
		void AddOnStateChangeCallback(const std::function<void(State, State)>& callback)
		{
			m_onStateChangeCallbacks.emplace_back(callback);
		}

		bool TrySetState(State state)
		{
			if (m_transitions[(int)m_currentState].end() != std::ranges::find(m_transitions[(int)m_currentState], state, &StateTransitionBase<State>::GetToState))
			{
				SetState(state);
				return true;
			}
			return false;
		}
	private:
		static constexpr const size_t StateCount = static_cast<size_t>(State::Size);

	private:
		State m_currentState;
		float m_timeInCurrentState = 0.0f;

		// 현재 상태에서 다른 상태로 전이할 수 있는 조건들 (단방향 연결 엣지)
		std::array<std::vector<std::unique_ptr<StateTransitionBase<State>>>, StateCount> m_transitions;

		// 상태 전이 조건 매개변수를 캐싱하는 컨테이너들
		std::map<std::string,int64_t> m_conditions;
		

		// 상태 전이 시 콜백 함수들
		// 매개변수: 이전 상태, 현재 상태
		std::vector<std::function<void(State, State)>> m_onStateChangeCallbacks;
	};
}