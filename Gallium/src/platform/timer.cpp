#include "timer_impl.h"

using namespace ga::platform;

Timer::Timer()
	: m_pImpl(std::make_unique<Impl>())
{
	QueryPerformanceFrequency(&m_pImpl->frequency);
	QueryPerformanceCounter(&m_pImpl->startTime);
	m_pImpl->lastTime = m_pImpl->startTime;
	m_pImpl->currentTime = m_pImpl->startTime;
}

void Timer::Tick()
{
	m_pImpl->lastTime = m_pImpl->currentTime;
	QueryPerformanceCounter(&m_pImpl->currentTime);

	const int64_t delta = m_pImpl->currentTime.QuadPart - m_pImpl->lastTime.QuadPart;
	const int64_t total = m_pImpl->currentTime.QuadPart - m_pImpl->startTime.QuadPart;

	m_pImpl->deltaTime   = static_cast<double>(delta) / static_cast<double>(m_pImpl->frequency.QuadPart);
	m_pImpl->elapsedTime = static_cast<double>(total) / static_cast<double>(m_pImpl->frequency.QuadPart);
}

double Timer::DeltaTime() const
{
	return m_pImpl->deltaTime;
}

double Timer::ElapsedTime() const
{
	return m_pImpl->elapsedTime;
}
