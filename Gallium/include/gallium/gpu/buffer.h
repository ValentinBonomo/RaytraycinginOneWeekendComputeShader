#ifndef GALLIUM__GPU__BUFFER_H
#define GALLIUM__GPU__BUFFER_H
#pragma once

#include <gallium/gpu/device.h>
#include <gallium/gpu/enums.h>

#include <array>
#include <memory>

namespace ga::gpu
{
	class  Device;
	struct DescriptorIndex;
	struct ShaderStructDesc;
	class  ShaderStructInstance;
	class  ShaderStructArrayInstance;
	class  TransferEncoder;

	struct BufferInfo
	{
		EBufferUsage usage;
		size_t       size;
		bool         createPersistentStaging = false;
		const void*  initialData             = nullptr;
	};

	class Buffer
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	
	public:
		Buffer(Device& device, const BufferInfo& info);

		template<typename T, size_t N>
		Buffer(Device& device, EBufferUsage usage, const std::array<T, N>& initialData)
			: Buffer(device, { .usage = usage, .size = N * sizeof(T), .initialData = initialData.data() })
		{}

		template<typename T>
		Buffer(Device& device, EBufferUsage usage, const std::vector<T>& initialData)
			: Buffer(device, { .usage = usage, .size = initialData.size() * sizeof(T), .initialData = initialData.data() })
		{}

		Buffer(Device& device, EBufferUsage usage, const ShaderStructDesc& shaderStruct, bool createPersistentStaging = false);
		Buffer(Device& device, EBufferUsage usage, const ShaderStructDesc& shaderStruct, size_t count, bool createPersistentStaging = false);
		Buffer(Device& device, EBufferUsage usage, const ShaderStructInstance& shaderStruct, bool createPersistentStaging = false);
		Buffer(Device& device, EBufferUsage usage, const ShaderStructArrayInstance& shaderStruct, bool createPersistentStaging = false);

		~Buffer();

		const Impl&            GetImpl() const;
		const DescriptorIndex& GetDescriptorIndex() const;
		uintptr_t              GetGpuAddress() const;

		void Upload(Device& device, const void* data, size_t size);
		SemaphoreId UploadAsync(const void* data, size_t size);
		void UploadInline(const ga::gpu::TransferEncoder& transfer, const void* data, size_t size);
		void Upload(Device& device, const ShaderStructInstance& data);
		SemaphoreId UploadAsync(const ShaderStructInstance& data);
		void UploadInline(const ga::gpu::TransferEncoder& transfer, const ShaderStructInstance& data);
		void Upload(Device& device, const ShaderStructArrayInstance& data);
		SemaphoreId UploadAsync(const ShaderStructArrayInstance& data);
		void UploadInline(const ga::gpu::TransferEncoder& transfer, const ShaderStructArrayInstance& data);

		template<typename T, size_t N>
		void Upload(Device& device, const std::array<T, N>& data)
		{
			Upload(device, data.data(), N * sizeof(T));
		}

		template<typename T>
		void Upload(Device& device, const std::vector<T>& data)
		{
			Upload(device, data.data(), data.size() * sizeof(T));
		}

		template<typename T, size_t N>
		SemaphoreId UploadAsync(const std::array<T, N>& data)
		{
			return UploadAsync(data.data(), N * sizeof(T));
		}

		template<typename T>
		SemaphoreId UploadAsync(const std::vector<T>& data)
		{
			return UploadAsync(data.data(), data.size() * sizeof(T));
		}

		template<typename T, size_t N>
		void UploadInline(const ga::gpu::TransferEncoder& transfer, const std::array<T, N>& data)
		{
			UploadInline(transfer, data.data(), N * sizeof(T));
		}

		template<typename T>
		void UploadInline(const ga::gpu::TransferEncoder& transfer, const std::vector<T>& data)
		{
			UploadInline(transfer, data.data(), data.size() * sizeof(T));
		}
	};
}

#endif /* GALLIUM__GPU__BUFFER_H */
