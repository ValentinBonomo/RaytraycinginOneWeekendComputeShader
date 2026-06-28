#pragma once

#include <gallium/gpu/image.h>
#include <gallium/gpu/descriptorregistry.h>
#include "device_impl.h"

namespace ga::gpu
{
	struct Image::Impl
	{
		Device*              owner;
		vk::raii::Image      image = nullptr;
		vk::Image            imageRaw;

		VmaAllocator         allocator  = VK_NULL_HANDLE;
		VmaAllocation        allocation = VK_NULL_HANDLE;

		vk::raii::ImageView  imageView = nullptr;
		vk::raii::Sampler    sampler   = nullptr;

		vk::ImageAspectFlags aspectMask;
		vk::Format           format;
		vk::ImageUsageFlags  usage;
		glm::uvec3           size;

		mutable vk::ImageLayout currentLayout;
		DescriptorIndex         descriptorIndex;
	};

	struct ExistingImageDesc
	{
		vk::Image            image;
		vk::ImageAspectFlags aspectMask;
		vk::Format           format;
		vk::ImageUsageFlags  usage;
		glm::uvec2           extent;
	};
}
