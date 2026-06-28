#ifndef GALLIUM__GPU__VULKAN__GRAPHICSPIPELINE_H
#define GALLIUM__GPU__VULKAN__GRAPHICSPIPELINE_H
#pragma once

#include <gallium/gpu/graphicspipeline.h>
#include "device_impl.h"

namespace ga::gpu
{
	struct GraphicsPipeline::Impl
	{
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline       pipeline       = nullptr;
	};
}

#endif /* GALLIUM__GPU__VULKAN__GRAPHICSPIPELINE_H */
