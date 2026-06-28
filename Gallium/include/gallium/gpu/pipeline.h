#ifndef GALLIUM__GPU__PIPELINE_H
#define GALLIUM__GPU__PIPELINE_H
#pragma once

#include <gallium/gpu/enums.h>
#include <gallium/gpu/shadermodule.h>

#include <string_view>

namespace ga::gpu
{
	struct PipelineStageDesc
	{
		ShaderModule&    module;
		EShaderStage     stage;
		std::string_view entrypoint;
	};
}

#endif /* GALLIUM__GPU__PIPELINE_H */
