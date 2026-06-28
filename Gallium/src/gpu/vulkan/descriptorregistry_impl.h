#ifndef GALLIUM__GPU__VULKAN__DESCRIPTORREGISTRY_H
#define GALLIUM__GPU__VULKAN__DESCRIPTORREGISTRY_H
#pragma once

#include <gallium/gpu/descriptorregistry.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <queue>
#include <vector>

namespace ga::gpu
{
	struct DescriptorRegistry::Impl
	{
		vk::raii::DescriptorPool      pool   = nullptr;
        vk::raii::DescriptorSetLayout layout = nullptr;
        vk::raii::DescriptorSet       set    = nullptr;

        std::queue<uint32_t> freeTextures;
        std::queue<uint32_t> freeRWTextures;
        std::queue<uint32_t> freeBuffers;

        uint32_t nextTexture   = 0;
        uint32_t nextRWTexture = 0;
        uint32_t nextBuffer    = 0;

        uint32_t AllocSlot(EDescriptorType type);
        void     FreeSlot(DescriptorIndex index);
	};
}

#endif /* GALLIUM__GPU__VULKAN__DESCRIPTORREGISTRY_H */