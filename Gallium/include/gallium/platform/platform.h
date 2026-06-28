#ifndef GALLIUM__PLATFORM__PLATFORM_H
#define GALLIUM__PLATFORM__PLATFORM_H
#pragma once

#include <memory>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace ga::platform
{
	class Vfs;
	class Input;
	class Timer;

	struct PlatformDesc
	{
		std::string appName;
		glm::uvec3  appVersion;
		glm::uvec2  requestedSurfaceSize;
		bool        requestFullscreen = false;
		bool        requestResizableWindow = false;
	};

	class Platform
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		struct Surface;

		Platform(const PlatformDesc& desc);
		~Platform();

		const Impl& GetImpl() const;

		Vfs& Vfs();
		Input& Input();
		Timer& Timer();

		glm::uvec2 SurfaceSize() const;
		
		void PollEvents();
		void RequestExit();
		bool IsExitRequested() const;
	};
}

#endif /* GALLIUM__PLATFORM__PLATFORM_H */
