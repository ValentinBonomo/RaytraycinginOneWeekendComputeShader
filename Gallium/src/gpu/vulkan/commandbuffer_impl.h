#pragma once

#include <gallium/gpu/commandbuffer.h>
#include "device_impl.h"

namespace ga::gpu
{
	struct CommandBuffer::Impl
	{
		vk::raii::CommandBuffer commandBuffer = nullptr;
		vk::raii::Semaphore     semaphore     = nullptr;
		vk::raii::Fence         isGpuFree     = nullptr;
		std::atomic_bool        isCpuFree     = false;
	};

	struct CommandEncoder::Impl
	{
		vk::raii::CommandBuffer* commandBuffer;
		vk::DescriptorSet        bindlessDescriptorSet;
	};

	struct RenderEncoder::Impl
	{
		vk::raii::CommandBuffer*        commandBuffer;
		const vk::raii::PipelineLayout* pipelineLayout;
	};

	struct ComputeEncoder::Impl
	{
		vk::raii::CommandBuffer*        commandBuffer;
		const vk::raii::PipelineLayout* pipelineLayout;
	};

	struct TransferEncoder::Impl
	{
		vk::raii::CommandBuffer* commandBuffer;
	};

	struct TransferEncoder::BufferCopyInfoInternal
	{
		vk::Buffer     source;
		vk::Buffer     destination;
		vk::BufferCopy region;
	};
}
