#ifndef GALLIUM__GPU__GRAPHICSPIPELINE_H
#define GALLIUM__GPU__GRAPHICSPIPELINE_H
#pragma once

#include <gallium/gpu/enums.h>
#include <gallium/gpu/pipeline.h>

#include <memory>
#include <optional>
#include <vector>

namespace ga::gpu
{
	class Device;

	struct VertexBindingDesc
	{
		uint32_t           binding;
		uint32_t           stride;
		EVertexBindingRate rate;
	};

	struct VertexAttributeDesc
	{
		uint32_t   location;
		uint32_t   binding;
		EFormat    format;
		uint32_t   offset;
	};

	struct VertexInputDesc
	{
		std::vector<VertexBindingDesc>   bindings;
		std::vector<VertexAttributeDesc> attributes;
	};

	struct AttachmentFormatsDesc
	{
		std::vector<EFormat>   colorFormats;
		std::optional<EFormat> depthFormat = std::nullopt;
	};

	struct ColorBlendDesc
	{
		bool         enabled        = false;
		EBlendFactor srcColorFactor = EBlendFactor::One;
		EBlendFactor dstColorFactor = EBlendFactor::Zero;
		EBlendOp     colorOp        = EBlendOp::Add;
		EBlendFactor srcAlphaFactor = EBlendFactor::One;
		EBlendFactor dstAlphaFactor = EBlendFactor::Zero;
		EBlendOp     alphaOp        = EBlendOp::Add;
	};

	struct GraphicsPipelineInfo
	{
		std::vector<PipelineStageDesc> stages;
		VertexInputDesc                vertexInput;
		AttachmentFormatsDesc          attachmentFormats;
		ColorBlendDesc                 blending;
	};

	class GraphicsPipeline
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		GraphicsPipeline(Device& device, const GraphicsPipelineInfo& info);
		~GraphicsPipeline();

		const Impl& GetImpl() const;
	};
}

#endif /* GALLIUM__GPU__GRAPHICSPIPELINE_H */
