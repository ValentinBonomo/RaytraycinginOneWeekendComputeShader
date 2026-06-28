#include "image_impl.h"
#include "enums_impl.h"

#include <gallium/gpu/buffer.h>

using namespace ga::gpu;

vk::raii::Sampler CreateSampler(const vk::raii::Device& device)
{
	auto createInfo = vk::SamplerCreateInfo {
		.magFilter        = vk::Filter::eLinear,
		.minFilter        = vk::Filter::eLinear,
		.mipmapMode       = vk::SamplerMipmapMode::eLinear,
		.addressModeU     = vk::SamplerAddressMode::eMirroredRepeat,
		.addressModeV     = vk::SamplerAddressMode::eMirroredRepeat,
		.addressModeW     = vk::SamplerAddressMode::eMirroredRepeat,
		.anisotropyEnable = true,
		.maxAnisotropy    = 4.0f
	};

	return device.createSampler(createInfo);
}

Image::Image(Device& device, const ImageDesc& desc)
	: m_pImpl(new Impl)
{
	m_pImpl->owner      = &device;
	m_pImpl->aspectMask = s_DeduceVkAspectMask(desc.format);
	m_pImpl->format     = s_ToVk(desc.format);
	m_pImpl->usage      = s_ToVk(desc.usage);

	auto imageCreateInfo = vk::ImageCreateInfo {
		.flags         = vk::ImageCreateFlags(0),
		.imageType     = s_ToVk(desc.type),
		.format        = s_ToVk(desc.format),
		.extent        = vk::Extent3D(uint32_t(desc.width), uint32_t(desc.height), uint32_t(desc.depth)),
		.mipLevels     = uint32_t(desc.levels),
		.arrayLayers   = uint32_t(desc.layers),
		.samples       = vk::SampleCountFlagBits::e1,
		.tiling        = vk::ImageTiling::eOptimal,
		.usage         = s_ToVk(desc.usage),
		.sharingMode   = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined
	};

	if (desc.initialData)
		imageCreateInfo.usage = imageCreateInfo.usage | vk::ImageUsageFlagBits::eTransferDst;

	auto allocInfo = VmaAllocationCreateInfo {
		.usage         = VMA_MEMORY_USAGE_AUTO,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	VkImage rawImage;
	vmaCreateImage(
		device.GetImpl().allocator,
		&*imageCreateInfo,
		&allocInfo,
		&rawImage,
		&m_pImpl->allocation,
		nullptr
	);

	m_pImpl->imageRaw = rawImage;
	m_pImpl->image    = vk::raii::Image(device.GetImpl().device, rawImage); // Transfer image ownership to the RAII object

	vk::ImageViewType viewType = ([](EImageType type) {
		switch (type)
		{
		case EImageType::Image1D: return vk::ImageViewType::e1D;
		case EImageType::Image2D: return vk::ImageViewType::e2D;
		case EImageType::Image3D: return vk::ImageViewType::e3D;
		}

		return vk::ImageViewType(-1);
	})(desc.type);

	auto imageViewCreateInfo = vk::ImageViewCreateInfo {
		.image    = m_pImpl->image,
		.viewType = viewType,
		.format   = s_ToVk(desc.format),
		.subresourceRange = {
			.aspectMask     = s_DeduceVkAspectMask(desc.format),
			.baseMipLevel   = 0,
			.levelCount     = uint32_t(desc.levels),
			.baseArrayLayer = 0,
			.layerCount     = uint32_t(desc.layers)
		}
	};

	m_pImpl->imageView     = vk::raii::ImageView(device.GetImpl().device, imageViewCreateInfo);
	m_pImpl->size          = { desc.width, desc.height, desc.depth };
	m_pImpl->allocator     = device.GetImpl().allocator;
	m_pImpl->currentLayout = vk::ImageLayout::eUndefined;
	m_pImpl->sampler       = CreateSampler(device.GetImpl().device);

	if (desc.initialData)
		Upload(device, {}, { desc.width, desc.height, desc.depth }, desc.initialData);

	if (desc.usage == EImageUsage::Sampled || desc.usage == EImageUsage::Storage)
		m_pImpl->descriptorIndex = device.GetDescriptorRegistry().Register(*this);

}

Image::Image(const Device& device, const ExistingImageDesc& desc)
	: m_pImpl(new Impl)
{
	m_pImpl->owner      = &const_cast<Device&>(device);
	m_pImpl->imageRaw   = desc.image;
	m_pImpl->aspectMask = s_DeduceVkAspectMask(s_FromVk(desc.format));
	m_pImpl->format     = desc.format;
	m_pImpl->usage      = desc.usage;
	m_pImpl->size       = glm::uvec3(desc.extent, 1);

	auto imageViewCreateInfo = vk::ImageViewCreateInfo {
		.image            = desc.image,
		.viewType         = vk::ImageViewType::e2D,
		.format           = desc.format,
		.subresourceRange = { desc.aspectMask, 0, 1, 0, 1 },
	};

	m_pImpl->imageView = vk::raii::ImageView(device.GetImpl().device, imageViewCreateInfo);
	m_pImpl->currentLayout = vk::ImageLayout::eUndefined;
}

Image::~Image()
{
	m_pImpl->owner->GetDescriptorRegistry().Unregister(m_pImpl->descriptorIndex);

	m_pImpl->imageView.clear();

	if (m_pImpl->image != nullptr)
	{
		if (m_pImpl->allocation)
			m_pImpl->image.release();
		else
			m_pImpl->image.clear();
	}
	
	if (m_pImpl->allocation)
		vmaDestroyImage(m_pImpl->allocator, m_pImpl->imageRaw, m_pImpl->allocation);
}

const Image::Impl& Image::GetImpl() const
{
	return *m_pImpl;
}

const DescriptorIndex& Image::GetDescriptorIndex() const
{
	return m_pImpl->descriptorIndex;
}

void Image::SetDescriptorIndex(const DescriptorIndex& descriptorIndex) const
{
	m_pImpl->descriptorIndex = descriptorIndex;
}

glm::uvec3 Image::Size() const
{
	return m_pImpl->size;
}

void Image::Upload(Device& device, const glm::uvec3& offset, const glm::uvec3& size, const void* pixels)
{
	auto stagingBuffer = std::make_unique<Buffer>(device, BufferInfo {
		.usage       = EBufferUsage::TransferSrc,
		.size        = size.x * size.y * size.z * s_ToVkPixelSize(m_pImpl->format),
		.initialData = pixels
	});

	auto cb = device.AcquireCommandBuffer();
	cb->Record([&](const CommandEncoder& encoder) {
		encoder.Transition(*this, ga::gpu::EImageLayout::TransferDstOptimal);
		encoder.Transfer([&](const TransferEncoder& transfer) {
			transfer.CopyBufferToImage(stagingBuffer.get(), this, 0, offset, size);
		});
		encoder.Transition(*this, ga::gpu::EImageLayout::ShaderReadOnlyOptimal);
	}, ECommandBufferUsage::OneTimeSubmit);
	device.SubmitAndWait(cb);
	device.ReleaseCommandBuffer(cb);
}
