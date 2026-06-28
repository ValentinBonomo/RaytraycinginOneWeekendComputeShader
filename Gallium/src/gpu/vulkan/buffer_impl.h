#ifndef GALLIUM__GPU__VULKAN__BUFFER_H
#define GALLIUM__GPU__VULKAN__BUFFER_H
#pragma once

#include <gallium/gpu/buffer.h>
#include <gallium/gpu/descriptorregistry.h>
#include "device_impl.h"

namespace ga::gpu
{
	struct Buffer::Impl
	{
		Device*              owner;
		vk::raii::Buffer     buffer     = nullptr;
		vk::DeviceAddress    gpuAddress;
		VmaAllocation        allocation = VK_NULL_HANDLE;
		VmaAllocationInfo    allocInfo;
		VmaAllocator         allocator  = VK_NULL_HANDLE;
		size_t               size       = 0;
		vk::BufferUsageFlags usage;
		DescriptorIndex      descriptorIndex;

		VmaAllocation        stagingAllocation = nullptr;
		vk::Buffer           stagingBuffer     = nullptr;
		void*                stagingMapped     = nullptr;
	};
}

#endif /* GALLIUM__GPU__VULKAN__BUFFER_H */
