#ifndef GALLIUM__GPU__VULKAN__COMPUTEPIPELINE_H
#define GALLIUM__GPU__VULKAN__COMPUTEPIPELINE_H
#pragma once

#include <gallium/gpu/computepipeline.h>
#include "device_impl.h"

namespace ga::gpu
{
	struct ComputePipeline::Impl
	{
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline       pipeline       = nullptr;
	};
}

#endif /* GALLIUM__GPU__VULKAN__COMPUTEPIPELINE_H */
