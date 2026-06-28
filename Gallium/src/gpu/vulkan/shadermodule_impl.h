#pragma once

#include <gallium/gpu/shadermodule.h>

#include "device_impl.h"
#include "shadermodule_reflection_impl.h"

namespace ga::gpu
{
	struct ShaderModule::Impl
	{
		vk::raii::ShaderModule     shaderModule = nullptr;
		ShaderModuleReflectionData reflectionData;
	};
}
