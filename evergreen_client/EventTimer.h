#pragma once

#include <queue>

class EventTimer
{
public:
	void Update(float deltaTime);
	void RegisterEvent(float seconds, std::function<void()> callback);
	void ClearEvents();

private:
	using EventPair = std::pair<float, std::function<void()>>;
	struct EventPairGreater
	{
		bool operator()(const EventPair& lhs, const EventPair& rhs) const
		{
			return lhs.first > rhs.first;
		}
	};
	float totalTime = 0.0f;
	std::priority_queue<EventPair, std::vector<EventPair>, EventPairGreater> m_events;
};