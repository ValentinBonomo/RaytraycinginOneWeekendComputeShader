#ifndef GALLIUM__GPU__VULKAN__SHADERMODULE_REFLECTION_IMPL_H
#define GALLIUM__GPU__VULKAN__SHADERMODULE_REFLECTION_IMPL_H
#pragma once

#include <gallium/gpu/shadermodule_reflection.h>
#include <SPIRV-Reflect/spirv_reflect.h>

namespace ga::gpu
{
	ShaderModuleReflectionData ReflectSpvShaderModule(size_t size, const void* data);
}

#endif /* GALLIUM__GPU__VULKAN__SHADERMODULE_REFLECTION_IMPL_H */
