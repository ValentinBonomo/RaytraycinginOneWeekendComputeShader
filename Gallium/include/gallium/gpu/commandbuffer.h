#ifndef GALLIUM__GPU__COMMANDBUFFER_H
#define GALLIUM__GPU__COMMANDBUFFER_H
#pragma once

#include <gallium/gpu/enums.h>

#include <glm/glm.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ga::gpu
{
	class Device;
	class Buffer;
	class Image;
	class GraphicsPipeline;
	class ComputePipeline;
	class ShaderStructInstance;

	class CommandEncoder;
	class RenderEncoder;
	class ComputeEncoder;
	class TransferEncoder;

	struct BeginRenderingInfo
	{
		GraphicsPipeline& pipeline;

		// Rendering area
		struct {
			glm::ivec2 offset;
			glm::uvec2 extent;
		} renderArea;

		uint32_t layerCount = 1;

		// Color attachments
		struct ColorAttachment
		{
			Image&    image;
			ELoadOp   loadOp     = ELoadOp::Clear;
			EStoreOp  storeOp    = EStoreOp::Store;
			glm::vec4 clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
		};

		std::vector<ColorAttachment> colorAttachments;

		// Depth/stencil attachment (optional)
		struct DepthAttachment
		{
			Image&   image;
			ELoadOp  loadOp     = ELoadOp::Clear;
			EStoreOp storeOp    = EStoreOp::Store;
			float    clearValue = 1.0f;
		};

		std::optional<DepthAttachment> depthAttachment = std::nullopt;
	};

	struct BeginComputeInfo
	{
		ComputePipeline& pipeline;
	};

	struct TransitionInfo
	{
		EPipelineStage srcStage;
		EAccessType    srcAccess;
		EPipelineStage dstStage;
		EAccessType    dstAccess;
		EImageLayout   dstLayout;
	};

	class CommandBuffer
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

		const Device& m_owner;

	public:
		explicit CommandBuffer(const Device& owner);
		~CommandBuffer();

		Impl& GetImpl() const;
		const Device& GetOwner() const;

		bool IsCpuFree() const;
		bool IsGpuFree() const;

		void WaitForCompletion();

		void Record(std::function<void(const CommandEncoder&)> contents, ECommandBufferUsage flags = ECommandBufferUsage::None);
	};

	class CommandEncoder
	{
	protected:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		CommandEncoder(CommandBuffer& cb);

		const Impl& GetImpl() const;

		void Transition(Image& image, const TransitionInfo& info) const;
		void Transition(Image& image, EImageLayout layout) const;

		void Render(const BeginRenderingInfo& info, std::function<void(const RenderEncoder&)> contents) const;
		void Compute(const BeginComputeInfo& info, std::function<void(const ComputeEncoder&)> contents) const;
		void Transfer(std::function<void(const TransferEncoder&)> contents) const;
	};

	class RenderEncoder
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		RenderEncoder(const CommandEncoder& parent, const BeginRenderingInfo& info);
		~RenderEncoder();

		RenderEncoder(const RenderEncoder&) = delete;
		RenderEncoder& operator=(const RenderEncoder&) = delete;

		void BindVertexBuffers(uint32_t firstBinding, const std::vector<std::pair<Buffer*, uint64_t /* offset */>>& buffers) const;
		void BindIndexBuffer(Buffer* buffer, uint64_t offset = 0) const;

		void PushConstants(uint32_t offset, uint32_t size, const void* pData) const;
		void PushConstants(const ShaderStructInstance& data) const;

		template<typename T, size_t N>
		void PushConstants(const std::array<T, N>& data) const
		{ PushConstants(0, N * sizeof(T), data.data()); }

		void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;

		void SetViewport(const glm::vec2& origin, const glm::vec2& size, const glm::vec2& zBounds = { 0.0f, 1.0f }) const;
		void SetScissor(const glm::ivec2& offset, const glm::uvec2& extent) const;
	};

	class ComputeEncoder
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		ComputeEncoder(const CommandEncoder& parent, const BeginComputeInfo& info);
		~ComputeEncoder();

		ComputeEncoder(const ComputeEncoder&) = delete;
		ComputeEncoder& operator=(const ComputeEncoder&) = delete;

		void PushConstants(uint32_t offset, uint32_t size, const void* pData) const;
		void PushConstants(const ShaderStructInstance& data) const;

		template<typename T, size_t N>
		void PushConstants(const std::array<T, N>& data) const
		{ PushConstants(0, N * sizeof(T), data.data()); }

		void Dispatch(const glm::uvec3& workgroups) const;
	};


	class TransferEncoder
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		struct BufferCopyInfoInternal;

		TransferEncoder(const CommandEncoder& parent);
		~TransferEncoder();

		TransferEncoder(const RenderEncoder&) = delete;
		TransferEncoder& operator=(const TransferEncoder&) = delete;

		void ClearColorImage(Image& image, const glm::vec4& color) const;
		void CopyImage(Image& src, Image& dst, const glm::uvec3& srcOffset, const glm::uvec3& dstOffset, const glm::uvec3& extent) const;
		void BlitImage(Image& src, Image& dst, const glm::uvec3& srcOffset, const glm::uvec3& dstOffset, const glm::uvec3& extent) const;

		void CopyBuffer(const BufferCopyInfoInternal& copyInfo) const;
		void CopyBuffer(Buffer* source, Buffer* destination, size_t sourceOffset, size_t destinationOffset, size_t size) const;

		void CopyBufferToImage(Buffer* source, Image* destination, size_t bufferOffset, const glm::uvec3& imageOffset, const glm::uvec3& imageExtent) const;
	
		void BufferBarrier(const Buffer& buffer) const;
	};
}

#endif /* GALLIUM__GPU__COMMANDBUFFER_H */




