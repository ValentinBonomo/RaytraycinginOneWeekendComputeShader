#include "buffer_impl.h"
#include "commandbuffer_impl.h"
#include "enums_impl.h"
#include "shadermodule_reflection_impl.h"

#include <tuple>

using namespace ga::gpu;

static std::tuple<vk::Buffer, VmaAllocation, void*> s_CreateStaging(VmaAllocator allocator, size_t size)
{
	const VkBufferCreateInfo stagingCI
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};

	const VmaAllocationCreateInfo stagingAllocCI
	{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	VmaAllocationInfo allocInfo;
	VkBuffer resultBuffer = VK_NULL_HANDLE;
	VmaAllocation resultAllocation = VK_NULL_HANDLE;
	vmaCreateBuffer(allocator, &stagingCI, &stagingAllocCI,
		&resultBuffer, &resultAllocation, &allocInfo);

	return std::make_tuple(resultBuffer, resultAllocation, allocInfo.pMappedData);
}

static void s_DestroyStaging(VmaAllocator allocator, VmaAllocation allocation, vk::Buffer buffer)
{
	vmaDestroyBuffer(allocator, buffer, allocation);
}

Buffer::Buffer(Device& device, const BufferInfo& info)
	: m_pImpl(new Impl)
{
	auto createInfo = vk::BufferCreateInfo {
		.size  = info.size,
		.usage = s_ToVk(info.usage) | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress
	};

	auto allocInfo = VmaAllocationCreateInfo {
		.flags = 0,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	VkBuffer buffer;
	vmaCreateBuffer(device.GetImpl().allocator, &*createInfo, &allocInfo, &buffer, &m_pImpl->allocation, &m_pImpl->allocInfo);
	m_pImpl->buffer    = vk::raii::Buffer(device.GetImpl().device, buffer); // Transfer buffer ownership to the RAII object
	m_pImpl->allocator = device.GetImpl().allocator;
	m_pImpl->size      = info.size;
	m_pImpl->usage     = s_ToVk(info.usage);

	if (info.initialData)
		Upload(device, info.initialData, info.size);

	m_pImpl->owner      = &device;
	m_pImpl->gpuAddress = device.GetImpl().device.getBufferAddress({ .buffer = *m_pImpl->buffer });

	if (info.createPersistentStaging)
		std::tie(
			m_pImpl->stagingBuffer,
			m_pImpl->stagingAllocation,
			m_pImpl->stagingMapped
		) = s_CreateStaging(m_pImpl->allocator, info.size);

	bool autoRegisterDescriptor = info.usage == EBufferUsage::StorageBuffer; // || EBufferUsage::UniformBuffer /* Kept commented for now as UBs could eventually go through GpuAddress */
	if (autoRegisterDescriptor)
		m_pImpl->descriptorIndex = device.GetDescriptorRegistry().Register(*this);
}	

Buffer::Buffer(Device& device, EBufferUsage usage, const ShaderStructDesc& shaderStruct, bool createPersistentStaging /* = false */)
	: Buffer(device, { .usage = usage, .size = shaderStruct.size, .createPersistentStaging = createPersistentStaging })
{}

Buffer::Buffer(Device& device, EBufferUsage usage, const ShaderStructDesc& shaderStruct, size_t count, bool createPersistentStaging /* = false */)
	: Buffer(device, { .usage = usage, .size = shaderStruct.size * count, .createPersistentStaging = createPersistentStaging })
{}

Buffer::Buffer(Device& device, EBufferUsage usage, const ShaderStructInstance& shaderStruct, bool createPersistentStaging /* = false */)
	: Buffer(device, { .usage = usage, .size = shaderStruct.Size(), .createPersistentStaging = createPersistentStaging, .initialData = shaderStruct.Data() })
{}

Buffer::Buffer(Device& device, EBufferUsage usage, const ShaderStructArrayInstance& shaderStruct, bool createPersistentStaging /* = false */)
	: Buffer(device, { .usage = usage, .size = shaderStruct.Size(), .createPersistentStaging = createPersistentStaging, .initialData = shaderStruct.Data() })
{}

Buffer::~Buffer()
{
	m_pImpl->owner->GetDescriptorRegistry().Unregister(m_pImpl->descriptorIndex);

	if (m_pImpl->stagingMapped)
		s_DestroyStaging(m_pImpl->allocator, m_pImpl->stagingAllocation, m_pImpl->stagingBuffer);

	vmaDestroyBuffer(m_pImpl->allocator, *m_pImpl->buffer, m_pImpl->allocation);
	m_pImpl->buffer.release();
}

const Buffer::Impl& Buffer::GetImpl() const
{
	return *m_pImpl;
}

const DescriptorIndex& Buffer::GetDescriptorIndex() const
{
	return m_pImpl->descriptorIndex;
}

uintptr_t Buffer::GetGpuAddress() const
{
	return m_pImpl->gpuAddress;
}

void Buffer::Upload(Device& device, const void* data, size_t size)
{
	auto createInfo = vk::BufferCreateInfo {
		.size  = size,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};

	auto allocCreateInfo = VmaAllocationCreateInfo {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};

	VkBuffer stagingBuffer;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VmaAllocationInfo allocationInfo;
	vmaCreateBuffer(m_pImpl->allocator, &*createInfo, &allocCreateInfo, &stagingBuffer, &allocation, &allocationInfo);

	void* ptr = nullptr;
	vmaMapMemory(m_pImpl->allocator, allocation, &ptr);
	std::copy_n(reinterpret_cast<const char*>(data), size, reinterpret_cast<char*>(ptr));
	vmaUnmapMemory(m_pImpl->allocator, allocation);

	auto commandBuffer = device.AcquireCommandBuffer();

	commandBuffer->Record([&](const CommandEncoder& encoder) {
		encoder.Transfer([&](const TransferEncoder& transfer) {
			transfer.CopyBuffer({ stagingBuffer, m_pImpl->buffer, { 0, 0, size } });
		});
	}, ECommandBufferUsage::OneTimeSubmit);

	device.SubmitAndWait(commandBuffer);
	device.ReleaseCommandBuffer(commandBuffer);

	vmaDestroyBuffer(m_pImpl->allocator, stagingBuffer, allocation);
}

SemaphoreId Buffer::UploadAsync(const void* data, size_t size)
{
	assert(m_pImpl->stagingMapped && "Buffer not created with staging");
	memcpy(m_pImpl->stagingMapped, data, size);

	auto cb = m_pImpl->owner->AcquireCommandBuffer();

	cb->Record([&](const CommandEncoder& encoder) {
		encoder.Transfer([&](const TransferEncoder& transfer) {
			transfer.CopyBuffer({ m_pImpl->stagingBuffer, m_pImpl->buffer, 0, 0, size });
		});
	});

	auto semaphore = m_pImpl->owner->Submit(cb);
	m_pImpl->owner->ReleaseCommandBuffer(cb);
	return *semaphore;
}

void Buffer::UploadInline(const ga::gpu::TransferEncoder& transfer, const void* data, size_t size)
{
	assert(m_pImpl->stagingMapped && "Buffer not created with staging");
	memcpy(m_pImpl->stagingMapped, data, size);

	transfer.CopyBuffer({ m_pImpl->stagingBuffer, m_pImpl->buffer, 0, 0, size });
	transfer.BufferBarrier(*this);
}

void Buffer::Upload(Device& device, const ShaderStructInstance& data)
{
	Upload(device, data.Data(), data.Size());
}

SemaphoreId Buffer::UploadAsync(const ShaderStructInstance& data)
{
	return UploadAsync(data.Data(), data.Size());
}

void Buffer::UploadInline(const ga::gpu::TransferEncoder& transfer, const ShaderStructInstance& data)
{
	UploadInline(transfer, data.Data(), data.Size());
}

void Buffer::Upload(Device& device, const ShaderStructArrayInstance& data)
{
	Upload(device, data.Data(), data.Size());
}

SemaphoreId Buffer::UploadAsync(const ShaderStructArrayInstance& data)
{
	return UploadAsync(data.Data(), data.Size());
}

void Buffer::UploadInline(const ga::gpu::TransferEncoder& transfer, const ShaderStructArrayInstance& data)
{
	UploadInline(transfer, data.Data(), data.Size());
}
