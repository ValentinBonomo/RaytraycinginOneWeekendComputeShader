#ifndef GALLIUM__PLATFORM__TIMER_H
#define GALLIUM__PLATFORM__TIMER_H
#pragma once

#include <cstdint>
#include <memory>

namespace ga::platform
{
	class Timer
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		Timer();
		~Timer();

		void   Reset();
		void   Tick();
		double DeltaTime() const;
		double ElapsedTime() const;
		void   SetElapsedTime(double t);
	};
}

#endif /* GALLIUM__PLATFORM__TIMER_H */