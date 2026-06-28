#ifndef GALLIUM__GPU__IMAGE_H
#define GALLIUM__GPU__IMAGE_H
#pragma once

#include <gallium/gpu/enums.h>

#include <glm/glm.hpp>
#include <memory>

namespace ga::gpu
{
	class  Device;
	struct DescriptorIndex;

	struct ImageDesc
	{
		EImageType  type;
		EFormat     format;
		size_t      width  = 0;
		size_t      height = 1;
		size_t      depth  = 1;
		EImageUsage usage  = EImageUsage(0);
		size_t      levels = 1;
		size_t      layers = 1;

		const void*     initialData = nullptr;
	};

	struct ExistingImageDesc;

	class Image
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		Image(Device& device, const ImageDesc& desc);
		Image(const Device& device, const ExistingImageDesc& desc);
		~Image();

		const Impl& GetImpl() const;
		const DescriptorIndex& GetDescriptorIndex() const;
		void SetDescriptorIndex(const DescriptorIndex& descriptorIndex) const;

		glm::uvec3  Size() const;
		void        Upload(Device& device, const glm::uvec3& offset, const glm::uvec3& size, const void* pixels);
	};
}

#endif /* GALLIUM__GPU__IMAGE_H */
