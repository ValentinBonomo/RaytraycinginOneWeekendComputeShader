#ifndef GALLIUM__GPU__VULKAN__DEVICE_H
#define GALLIUM__GPU__VULKAN__DEVICE_H
#pragma once

#include <gallium/gpu/device.h>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vma/vk_mem_alloc.h>

#include "../../platform/win32/platform_impl.h"
#include "commandbuffer_impl.h"
#include "descriptorregistry_impl.h"

#include <memory>
#include <mutex>
#include <unordered_map>

struct GLFWwindow;

namespace ga::gpu
{
	class Image;

	struct FrameData
	{
		vk::raii::Semaphore                          presentCompleteSemaphore    = nullptr;
		vk::raii::Semaphore                          renderFinishedSemaphore     = nullptr;
		vk::raii::Fence                              inFlightFence               = nullptr;
		std::vector<std::move_only_function<void()>> deferredActions;
	};

	struct Device::Impl
	{
		GLFWwindow*                         window;
		vk::raii::Instance                  instance                = nullptr;
		vk::raii::PhysicalDevice            physicalDevice          = nullptr;
		uint32_t                            mainQueueFamily         = uint32_t(-1);
		vk::raii::Queue                     mainQueue               = nullptr;
		vk::raii::SurfaceKHR                surface                 = nullptr;
		vk::raii::Device                    device                  = nullptr;
		vk::raii::SwapchainKHR              swapchain               = nullptr;
		std::vector<std::unique_ptr<Image>> swapchainImages;
		bool                                swapchainNeedsRecreation = false;
		VmaAllocator                        allocator               = VK_NULL_HANDLE;
		vk::raii::DescriptorPool            descriptorPool          = nullptr;
		vk::raii::CommandPool               commandPool             = nullptr;

		std::unique_ptr<DescriptorRegistry> descriptorRegistry = nullptr;

		std::vector<FrameData>              frameData;
		size_t                              currentFrame       = 0;
		uint32_t                            currentImageIndex  = 0;

		std::unordered_map<CommandBuffer*, std::unique_ptr<CommandBuffer>> commandBuffers;
		std::mutex                                                         commandBufferMutex;

		using CreateSwapchainReturnType = std::pair<vk::raii::SwapchainKHR, std::vector<std::unique_ptr<Image>>>;

		vk::raii::Instance        InitInstance(const char* appName, const glm::uvec3& appVersion);
		vk::raii::PhysicalDevice  PickPhysicalDevice();
		uint32_t                  PickQueueFamily(vk::QueueFlags requestedQueueFlags);
		vk::raii::SurfaceKHR      CreateGlfwWindowSurface(GLFWwindow* window);
		vk::raii::Device          CreateDevice(uint32_t graphicsQueueFamily);
		CreateSwapchainReturnType CreateSwapchain(const Device& owner, GLFWwindow* window);
		void                      RecreateSwapchain(const Device& owner, GLFWwindow* window);
		vk::raii::DescriptorPool  CreateDescriptorPool();
	};
}

ga::gpu::SemaphoreId s_FromVk(VkSemaphore semaphore);
VkSemaphore s_ToVk(ga::gpu::SemaphoreId id);

#endif /* GALLIUM__GPU__VULKAN__DEVICE_H */
