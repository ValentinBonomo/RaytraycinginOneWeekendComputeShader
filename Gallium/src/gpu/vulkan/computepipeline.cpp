#include "computepipeline_impl.h"
#include "enums_impl.h"
#include "shadermodule_impl.h"

using namespace ga::gpu;


ComputePipeline::ComputePipeline(Device& device, const ComputePipelineInfo& info)
	: m_pImpl(new Impl)
{
	std::optional<vk::PushConstantRange> pushConstantRange = std::nullopt;

	if (auto pc = info.stage.module.GetStruct("PushConstants"); pc)
	{
		pushConstantRange = vk::PushConstantRange {
			.stageFlags = vk::ShaderStageFlagBits::eAll,
			.offset     = 0u,
			.size       = pc->size
		};
	}

	auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo {
		.setLayoutCount         = 1u,
		.pSetLayouts            = &*device.GetDescriptorRegistry().GetImpl().layout,
		.pushConstantRangeCount = pushConstantRange.has_value() ? 1u : 0u,
		.pPushConstantRanges    = pushConstantRange.has_value() ? &*pushConstantRange : nullptr
	};
	m_pImpl->pipelineLayout = vk::raii::PipelineLayout(device.GetImpl().device, pipelineLayoutInfo);

	auto pipelineInfo = vk::ComputePipelineCreateInfo {
		.stage = vk::PipelineShaderStageCreateInfo {
			.stage  = s_ToVk(info.stage.stage),
			.module = info.stage.module.GetImpl().shaderModule,
			.pName  = info.stage.entrypoint.data()
		},
		.layout = m_pImpl->pipelineLayout
	};

	m_pImpl->pipeline = vk::raii::Pipeline(device.GetImpl().device, nullptr, pipelineInfo);
}

ComputePipeline::~ComputePipeline()
{
	m_pImpl->pipeline.clear();
	m_pImpl->pipelineLayout.clear();
}

const ComputePipeline::Impl& ComputePipeline::GetImpl() const
{
	return *m_pImpl;
}

