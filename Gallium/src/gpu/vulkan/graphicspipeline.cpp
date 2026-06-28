#include "graphicspipeline_impl.h"
#include "enums_impl.h"
#include "shadermodule_impl.h"

#include <ranges>

using namespace ga::gpu;

GraphicsPipeline::GraphicsPipeline(Device& device, const GraphicsPipelineInfo& info)
	: m_pImpl(new Impl)
{
	auto colorBlendAttachments = info.attachmentFormats.colorFormats | std::views::transform([&](const EFormat&) {
		return vk::PipelineColorBlendAttachmentState{
			.blendEnable         = info.blending.enabled,
			.srcColorBlendFactor = s_ToVk(info.blending.srcColorFactor),
			.dstColorBlendFactor = s_ToVk(info.blending.dstColorFactor),
			.colorBlendOp        = s_ToVk(info.blending.colorOp),
			.srcAlphaBlendFactor = s_ToVk(info.blending.srcAlphaFactor),
			.dstAlphaBlendFactor = s_ToVk(info.blending.dstAlphaFactor),
			.alphaBlendOp        = s_ToVk(info.blending.alphaOp),
			.colorWriteMask      = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags,
		};


		return vk::PipelineColorBlendAttachmentState {  };
	}) | std::ranges::to<std::vector>();

	auto dynamicStates = std::vector<vk::DynamicState> { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo {
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates    = dynamicStates.data()
	};

	const auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo { .topology = vk::PrimitiveTopology::eTriangleList };
	const auto viewportState = vk::PipelineViewportStateCreateInfo      { .viewportCount = 1, .scissorCount = 1 };
	const auto rasterizer    = vk::PipelineRasterizationStateCreateInfo { .lineWidth = 1.0f };
	const auto multisampling = vk::PipelineMultisampleStateCreateInfo   {};
	
	const auto blendState = vk::PipelineColorBlendStateCreateInfo {
		.logicOpEnable   = false,
		.attachmentCount = uint32_t(colorBlendAttachments.size()),
		.pAttachments    = colorBlendAttachments.data(),
	};
	
	const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo {
		.depthTestEnable       = true,
		.depthWriteEnable      = true,
		.depthCompareOp        = vk::CompareOp::eLess,
		.depthBoundsTestEnable = false,
		.maxDepthBounds        = 1.0f
	};

	auto colorFormats = info.attachmentFormats.colorFormats | std::views::transform([](EFormat f) { return s_ToVk(f); }) | std::ranges::to<std::vector>();

	auto pipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo {
		.colorAttachmentCount    = uint32_t(info.attachmentFormats.colorFormats.size()),
		.pColorAttachmentFormats = colorFormats.data(),
		.depthAttachmentFormat   = info.attachmentFormats.depthFormat ? s_ToVk(*info.attachmentFormats.depthFormat) : vk::Format::eUndefined
	};

	auto pipelineStages = info.stages | std::views::transform([](const PipelineStageDesc& s) {
		return vk::PipelineShaderStageCreateInfo {
			.stage  = s_ToVk(s.stage),
			.module = s.module.GetImpl().shaderModule,
			.pName  = s.entrypoint.data()
		};
	}) | std::ranges::to<std::vector>();

	auto vertexBindings = info.vertexInput.bindings | std::views::transform([](const ga::gpu::VertexBindingDesc& b) {
		return vk::VertexInputBindingDescription {
			.binding   = b.binding,
			.stride    = b.stride,
			.inputRate = s_ToVk(b.rate)
		};
	}) | std::ranges::to<std::vector>();

	auto vertexAttribs = info.vertexInput.attributes | std::views::transform([](const auto& a) {
		return vk::VertexInputAttributeDescription {
			.location = a.location,
			.binding  = a.binding,
			.format   = s_ToVk(a.format),
			.offset   = a.offset
		};
	}) | std::ranges::to<std::vector>();

	auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo {
		.vertexBindingDescriptionCount   = uint32_t(vertexBindings.size()),
		.pVertexBindingDescriptions      = vertexBindings.data(),
		.vertexAttributeDescriptionCount = uint32_t(vertexAttribs.size()),
		.pVertexAttributeDescriptions    = vertexAttribs.data()
	};

	std::optional<vk::PushConstantRange> pushConstantRange = std::nullopt;

	for (auto& s : info.stages)
	{
		if (auto pc = s.module.GetStruct("PushConstants"); pc)
		{
			pushConstantRange = vk::PushConstantRange {
				.stageFlags = vk::ShaderStageFlagBits::eAll,
				.offset     = 0u,
				.size       = pc->size
			};

			break;
		}
	}

	auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo {
		.setLayoutCount         = 1u,
		.pSetLayouts            = &*device.GetDescriptorRegistry().GetImpl().layout,
		.pushConstantRangeCount = pushConstantRange.has_value() ? 1u : 0u,
		.pPushConstantRanges    = pushConstantRange.has_value() ? &*pushConstantRange : nullptr
	};
	m_pImpl->pipelineLayout = vk::raii::PipelineLayout(device.GetImpl().device, pipelineLayoutInfo);

	auto pipelineInfo = vk::GraphicsPipelineCreateInfo {
		.pNext      = &pipelineRenderingCreateInfo,
		.stageCount = uint32_t(pipelineStages.size()),
		.pStages    = pipelineStages.data(),

		.pVertexInputState   = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState      = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState   = &multisampling,
		.pDepthStencilState  = info.attachmentFormats.depthFormat ? &depthStencilState : nullptr,
		.pColorBlendState    = &blendState,
		.pDynamicState       = &dynamicStateCreateInfo,
		.layout              = m_pImpl->pipelineLayout,
		.renderPass          = nullptr
	};

	m_pImpl->pipeline  = vk::raii::Pipeline(device.GetImpl().device, nullptr, pipelineInfo);
}

GraphicsPipeline::~GraphicsPipeline()
{
	m_pImpl->pipeline.clear();
	m_pImpl->pipelineLayout.clear();
}

const GraphicsPipeline::Impl& GraphicsPipeline::GetImpl() const
{
	return *m_pImpl;
}
