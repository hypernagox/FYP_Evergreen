#include "pch.h"

namespace Common
{
	// 애니메이션 상태 전이 클래스
	// 애니메이션이 끝나면 상태 전이를 수행
	template <class State>
	class AnimationStateTransition : public StateTransitionBase<State>
	{
	public:
		AnimationStateTransition(State toState, udsdx::RiggedMeshRenderer* target) : StateTransitionBase<State>(toState), m_target(target)
		{
		}

		bool IsTriggered(float time) const override
		{
			return !m_target->IsAnimationPlaying();
		}

	private:
		udsdx::RiggedMeshRenderer* m_target;
	};
}