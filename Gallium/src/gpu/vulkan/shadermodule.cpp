#include "shadermodule_impl.h"

using namespace ga::gpu;

#include <print>
#include <string_view>
#include <memory>

ShaderModule::ShaderModule(const Device& device, const void* data, size_t size)
	: m_pImpl(new Impl)
{
	auto createInfo = vk::ShaderModuleCreateInfo {
		.codeSize = size,
		.pCode    = reinterpret_cast<const uint32_t*>(data)
	};

	m_pImpl->shaderModule = vk::raii::ShaderModule(device.GetImpl().device, createInfo);
	m_pImpl->reflectionData = ReflectSpvShaderModule(size, data);
}

ShaderModule::ShaderModule(ShaderModule&& other)
	: m_pImpl(new Impl)
{
	m_pImpl->shaderModule = std::move(other.m_pImpl->shaderModule);
}

ShaderModule::~ShaderModule()
{
	m_pImpl->shaderModule.clear();
}

const ShaderModule::Impl& ShaderModule::GetImpl() const
{
	return *m_pImpl;
}

bool ShaderModule::HasStruct(std::string_view name)
{
	return m_pImpl->reflectionData.contains(std::string(name));
}

const ShaderStructDesc* ShaderModule::GetStruct(std::string_view name)
{
	if (HasStruct(name))
		return m_pImpl->reflectionData[std::string(name)].get();

	return nullptr;
}
