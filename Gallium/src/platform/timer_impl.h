#ifndef GALLIUM__PLATFORM__WIN32__TIMER_IMPL_H
#define GALLIUM__PLATFORM__WIN32__TIMER_IMPL_H
#pragma once

#include <gallium/platform/timer.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>

namespace ga::platform
{
	struct Timer::Impl
	{
		LARGE_INTEGER frequency   = {};
		LARGE_INTEGER startTime   = {};
		LARGE_INTEGER lastTime    = {};
		LARGE_INTEGER currentTime = {};

		double        deltaTime   = 0.0;
		double        elapsedTime = 0.0;
	};
}

#endif /* GALLIUM__PLATFORM__WIN32__TIMER_IMPL_H */