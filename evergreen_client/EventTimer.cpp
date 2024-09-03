#include "pch.h"
#include "EventTimer.h"

void EventTimer::Update(float deltaTime)
{
	totalTime += deltaTime;

	while (m_events.size() > 0 && m_events.top().first <= totalTime)
	{
		m_events.top().second();
		m_events.pop();
	}
}

void EventTimer::RegisterEvent(float seconds, std::function<void()> callback)
{
	m_events.push(std::make_pair(totalTime + seconds, callback));
}

void EventTimer::ClearEvents()
{
	while (m_events.size() > 0)
	{
		m_events.pop();
	}
}