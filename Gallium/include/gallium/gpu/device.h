#ifndef GALLIUM__GPU__DEVICE_H
#define GALLIUM__GPU__DEVICE_H
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ga::platform { class Platform; }

namespace ga::gpu
{
	class CommandBuffer;
	class DescriptorRegistry;
	class Image;

	enum class SemaphoreId : uintptr_t {};

	class Device
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		explicit Device(const platform::Platform& platform);
		~Device();

		const Impl& GetImpl() const;
		uint32_t    FrameCount() const;

		DescriptorRegistry& GetDescriptorRegistry();

		CommandBuffer* AcquireCommandBuffer();
		void ReleaseCommandBuffer(CommandBuffer*& commandBuffer);

		std::optional<SemaphoreId> Submit(CommandBuffer* commandBuffer, const std::vector<SemaphoreId>& waitOps = {}, std::optional<SemaphoreId> signalOp = std::nullopt);
		void SubmitAndWait(CommandBuffer* commandBuffer, const std::vector<SemaphoreId>& waitOps = {}) const;
		void WaitIdle();

		void Defer(std::move_only_function<void()> action);

		struct AcquiredImage
		{
			Image&      image;
			SemaphoreId presentComplete; // Wait on this when you first write to the image
			SemaphoreId renderComplete;  // Signal this on your last submission
		};

		AcquiredImage AcquireNextImage();
		void Present(const std::vector<SemaphoreId>& waitOps);
	};
}

#endif /* GALLIUM__GPU__DEVICE_H */
