#ifndef GALLIUM__GPU__SHADERMODULE_H
#define GALLIUM__GPU__SHADERMODULE_H
#pragma once

#include <memory>
#include <string_view>

namespace ga::gpu
{
	class  Device;
	struct ShaderStructDesc;

	class ShaderModule
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		ShaderModule(const Device& device, const void* data, size_t size);
		ShaderModule(ShaderModule&& other);
		~ShaderModule();

		const Impl& GetImpl() const;

		bool HasStruct(std::string_view name);
		const ShaderStructDesc* GetStruct(std::string_view name);
	};
}

#endif /* GALLIUM__GPU__SHADERMODULE_H */
