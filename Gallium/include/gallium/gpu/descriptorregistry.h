#ifndef GALLIUM__GPU__DESCRIPTORREGISTRY_H
#define GALLIUM__GPU__DESCRIPTORREGISTRY_H
#pragma once

#include <memory>

namespace ga::gpu
{
	class Device;
	class Buffer;
	class Image;

	constexpr uint32_t kMaxBindlessTextures = 16384;
	constexpr uint32_t kMaxBindlessBuffers  = 16384;
	constexpr uint32_t kInvalidDescriptor   = UINT32_MAX;

	enum class EDescriptorType
		: uint8_t
	{
		Texture,
		RWTexture,
		Buffer,
		Invalid = 0xff
	};

	struct DescriptorIndex
	{
		uint32_t        slot = kInvalidDescriptor;
		EDescriptorType type = EDescriptorType::Invalid;

		bool IsValid() const { return slot != kInvalidDescriptor; }
	};

	class DescriptorRegistry
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		DescriptorRegistry(const Device& device);
		~DescriptorRegistry();

		DescriptorRegistry(const DescriptorRegistry&)            = delete;
		DescriptorRegistry& operator=(const DescriptorRegistry&) = delete;

		const Impl& GetImpl() const;

		DescriptorIndex Register(const Image& image);
		DescriptorIndex Register(const Buffer& buffer);

		void Unregister(DescriptorIndex& index);
	};
}

#endif /* GALLIUM__GPU__DESCRIPTORREGISTRY_H */
