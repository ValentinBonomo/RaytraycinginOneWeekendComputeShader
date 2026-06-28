#define VMA_IMPLEMENTATION
#include "device_impl.h"
#include "commandbuffer_impl.h"
#include "image_impl.h"

#include <array>
#include <print>
#include <ranges>

using namespace ga::gpu;

const std::array  gValidationLayers = { "VK_LAYER_KHRONOS_validation" };
vk::raii::Context gVkRaiiContext;

ga::gpu::SemaphoreId s_FromVk(VkSemaphore semaphore)
{
	return static_cast<SemaphoreId>(reinterpret_cast<uintptr_t>(semaphore));
}

VkSemaphore s_ToVk(ga::gpu::SemaphoreId id)
{
	return reinterpret_cast<VkSemaphore>(static_cast<uintptr_t>(id));
}

vk::raii::Instance Device::Impl::InitInstance(const char* appName, const glm::uvec3& appVersion)
{
	const auto appInfo = vk::ApplicationInfo {
		.pApplicationName   = appName,
		.applicationVersion = VK_MAKE_VERSION(appVersion.x, appVersion.y, appVersion.z),
		.pEngineName        = "Gallium",
		.engineVersion      = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion         = vk::ApiVersion14
	};

	std::vector<char const*> requiredLayers;
#ifndef NDEBUG
	requiredLayers.assign(gValidationLayers.begin(), gValidationLayers.end());
#endif /* NDEBUG */

	const auto layerProperties = gVkRaiiContext.enumerateInstanceLayerProperties();
	if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) { return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; }); }))
		throw std::runtime_error("One or more required layers are not supported !");

	uint32_t glfwExtensionCount  = 0;
	auto     glfwExtensions      = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	auto     extensionProperties = gVkRaiiContext.enumerateInstanceExtensionProperties();

	std::vector<const char*> activeExtensions;
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		activeExtensions.push_back(glfwExtensions[i]);

	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
		if (std::ranges::none_of(extensionProperties, [glfwExtension = glfwExtensions[i]](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
			throw std::runtime_error("Required GLFW extensions not supported");

	auto instanceCreateInfo = vk::InstanceCreateInfo {
		.pApplicationInfo        = &appInfo,
		.enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames     = requiredLayers.data(),
		.enabledExtensionCount   = uint32_t(activeExtensions.size()),
		.ppEnabledExtensionNames = activeExtensions.data()
	};

	std::println("Gallium: Initialized Vulkan instance");
	return vk::raii::Instance(gVkRaiiContext, instanceCreateInfo);
}

vk::raii::PhysicalDevice Device::Impl::PickPhysicalDevice()
{
	auto devices = instance.enumeratePhysicalDevices();
	if (devices.empty())
		return nullptr;

	vk::raii::PhysicalDevice result = nullptr;

	for (const auto& d : devices)
	{
		auto deviceProperties = d.getProperties();

		if (deviceProperties.apiVersion < VK_API_VERSION_1_4)
			continue;

		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			return d;

		result = d;
	}

	return result;
}

uint32_t Device::Impl::PickQueueFamily(vk::QueueFlags requestedQueueFlags)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	auto graphicsQueueFamilyProperty = std::find_if(
		queueFamilyProperties.begin(),
		queueFamilyProperties.end(),
		[&](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & requestedQueueFlags; }
	);

	auto result = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	std::println("Gallium: Picked queue family = {}", result);
	return result;
}

vk::raii::SurfaceKHR Device::Impl::CreateGlfwWindowSurface(GLFWwindow* window)
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GLFW window surface");

	return vk::raii::SurfaceKHR(instance, _surface);
}

vk::raii::Device Device::Impl::CreateDevice(uint32_t graphicsQueueFamily)
{
	float queuePriority = 0.0f;
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
		.queueFamilyIndex = graphicsQueueFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	vk::StructureChain featureChain = {
		vk::PhysicalDeviceFeatures2 {
			.features = {
				.samplerAnisotropy = true,
				.shaderInt64       = true
			}
		},
		vk::PhysicalDeviceRobustness2FeaturesEXT {
			.nullDescriptor = true
		},	
		vk::PhysicalDeviceVulkan12Features {
			.descriptorIndexing                            = true,
			.shaderSampledImageArrayNonUniformIndexing     = true,
			.shaderStorageBufferArrayNonUniformIndexing    = true,
			.shaderStorageImageArrayNonUniformIndexing     = true,
			.descriptorBindingSampledImageUpdateAfterBind  = true,
			.descriptorBindingStorageImageUpdateAfterBind  = true,
			.descriptorBindingStorageBufferUpdateAfterBind = true,
			.descriptorBindingUpdateUnusedWhilePending     = true,
			.descriptorBindingPartiallyBound               = true,
			.runtimeDescriptorArray                        = true,
			.bufferDeviceAddress                           = true
		},
		vk::PhysicalDeviceVulkan13Features {
			.synchronization2 = true,
			.dynamicRendering = true
		}
	};

	const std::array deviceExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName
	};

	auto deviceCreateInfo = vk::DeviceCreateInfo {
		.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount    = 1,
		.pQueueCreateInfos       = &deviceQueueCreateInfo,
		.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	return vk::raii::Device(physicalDevice, deviceCreateInfo);
}

Device::Impl::CreateSwapchainReturnType Device::Impl::CreateSwapchain(const Device& owner, GLFWwindow* window)
{
	auto surfaceCaps           = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto availableFormats      = physicalDevice.getSurfaceFormatsKHR(surface);
	auto availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	auto swapSurfaceFormat = ([](const decltype(availableFormats)& formats) {
		for (const auto& availableFormat : formats)
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return availableFormat;
		return formats[0];
	})(availableFormats);

	auto swapExtent = ([&window](const vk::SurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		return vk::Extent2D {
			std::clamp<uint32_t>(width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	})(surfaceCaps);

	auto swapPresentMode = ([](const decltype(availablePresentModes)& modes) {
		for (const auto& availablePresentMode : modes)
			if (availablePresentMode == vk::PresentModeKHR::eFifoRelaxed)
				return availablePresentMode;
		return vk::PresentModeKHR::eFifo;
	})(availablePresentModes);

	auto minImageCount = std::clamp(3u, surfaceCaps.minImageCount, surfaceCaps.maxImageCount);

	auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR {
		.flags            = vk::SwapchainCreateFlagsKHR(0),
		.surface          = surface,
		.minImageCount    = minImageCount,
		.imageFormat      = swapSurfaceFormat.format,
		.imageColorSpace  = swapSurfaceFormat.colorSpace,
		.imageExtent      = swapExtent,
		.imageArrayLayers = 1,
		.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform     = surfaceCaps.currentTransform,
		.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode      = swapPresentMode,
		.clipped          = true,
		.oldSwapchain     = (swapchain != nullptr) ? *swapchain : nullptr
	};

	auto swapchain = vk::raii::SwapchainKHR(device, swapchainCreateInfo);

	auto swapchainImages = swapchain.getImages() | std::views::transform([&owner, swapSurfaceFormat, swapExtent](const vk::Image& image) {
		auto desc = ExistingImageDesc {
			.image      = image,
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.format     = swapSurfaceFormat.format,
			.usage      = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
			.extent     = glm::uvec2(swapExtent.width, swapExtent.height)
		};

		return std::make_unique<Image>(owner, desc);
	}) | std::ranges::to<std::vector>();

	return { std::move(swapchain), std::move(swapchainImages) };
}

void Device::Impl::RecreateSwapchain(const Device& owner, GLFWwindow* window)
{
	// Wait out minimization
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();
	
	auto [newSwapchain, newImages] = CreateSwapchain(owner, window);
	swapchain                      = std::move(newSwapchain);
	swapchainImages                = std::move(newImages);
	swapchainNeedsRecreation       = false;
}

vk::raii::DescriptorPool Device::Impl::CreateDescriptorPool()
{
	std::vector<vk::DescriptorPoolSize> poolSizes = {
		{ vk::DescriptorType::eCombinedImageSampler, 4096 },
		{ vk::DescriptorType::eUniformBuffer,        2048 },
		{ vk::DescriptorType::eStorageBuffer,        2048 },
		{ vk::DescriptorType::eStorageImage,         1024 }
	};

	auto createInfo = vk::DescriptorPoolCreateInfo{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 256,
		.poolSizeCount = uint32_t(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	return device.createDescriptorPool(createInfo);
}

// ----------------------------------------------------------------------------

Device::Device(const platform::Platform& platform)
	: m_pImpl(new Impl)
{
	m_pImpl->window         = platform.GetImpl().surface.window;
	m_pImpl->instance       = m_pImpl->InitInstance(platform.GetImpl().desc.appName.c_str(), platform.GetImpl().desc.appVersion);
	m_pImpl->physicalDevice = m_pImpl->PickPhysicalDevice();
	if (m_pImpl->physicalDevice != nullptr)
		std::println("Gallium: Picked PhysicalDevice = {}", static_cast<std::string_view>(m_pImpl->physicalDevice.getProperties().deviceName));
	else
		throw std::runtime_error("Unable to pick a physical device.");

	m_pImpl->mainQueueFamily = m_pImpl->PickQueueFamily(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
	m_pImpl->surface         = m_pImpl->CreateGlfwWindowSurface(platform.GetImpl().surface.window);
	m_pImpl->device          = m_pImpl->CreateDevice(m_pImpl->mainQueueFamily);
	m_pImpl->mainQueue       = vk::raii::Queue(m_pImpl->device, m_pImpl->mainQueueFamily, 0); // Fetches a queue from the device (the only one it has in this case)

	std::tie(
		m_pImpl->swapchain,
		m_pImpl->swapchainImages
	) = m_pImpl->CreateSwapchain(*this, platform.GetImpl().surface.window);

	auto vmaCreateInfo = VmaAllocatorCreateInfo {
		.flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice   = *m_pImpl->physicalDevice,
		.device           = *m_pImpl->device,
		.instance         = *m_pImpl->instance,
		.vulkanApiVersion = VK_MAKE_VERSION(1, 4, 0)
	};
	vmaCreateAllocator(&vmaCreateInfo, &m_pImpl->allocator);

	auto poolInfo = vk::CommandPoolCreateInfo {
		.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = m_pImpl->mainQueueFamily
	};
	m_pImpl->commandPool    = vk::raii::CommandPool(m_pImpl->device, poolInfo);
	m_pImpl->descriptorPool = m_pImpl->CreateDescriptorPool();

	m_pImpl->frameData = m_pImpl->swapchainImages | std::views::transform([&](const std::unique_ptr<Image>&) {
		auto allocInfo = vk::CommandBufferAllocateInfo {
			.commandPool        = *m_pImpl->commandPool,
			.level              = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		return FrameData {
			.presentCompleteSemaphore = vk::raii::Semaphore(m_pImpl->device, vk::SemaphoreCreateInfo()),
			.renderFinishedSemaphore  = vk::raii::Semaphore(m_pImpl->device, vk::SemaphoreCreateInfo()),
			.inFlightFence            = vk::raii::Fence(m_pImpl->device, {.flags = vk::FenceCreateFlagBits::eSignaled })
		};
	}) | std::ranges::to<std::vector>();

	m_pImpl->descriptorRegistry = std::make_unique<DescriptorRegistry>(*this);
}

Device::~Device()
{
	WaitIdle();

	m_pImpl->frameData.clear();

	m_pImpl->descriptorRegistry.reset();
	m_pImpl->descriptorPool.clear();
	m_pImpl->commandBuffers.clear();
	m_pImpl->commandPool.clear();
	vmaDestroyAllocator(m_pImpl->allocator);
	m_pImpl->swapchainImages.clear();
	m_pImpl->swapchain.clear();
	m_pImpl->surface.clear();
	m_pImpl->device.clear();
	m_pImpl->mainQueue.clear();
	m_pImpl->physicalDevice.clear();
	m_pImpl->instance.clear();
}

const Device::Impl& Device::GetImpl() const
{
	return *m_pImpl;
}

uint32_t Device::FrameCount() const
{
	return uint32_t(m_pImpl->swapchainImages.size());
}

DescriptorRegistry& Device::GetDescriptorRegistry()
{
	return *m_pImpl->descriptorRegistry;
}

CommandBuffer* Device::AcquireCommandBuffer()
{
	std::scoped_lock lock(m_pImpl->commandBufferMutex);

	for (auto& [_, cb] : m_pImpl->commandBuffers)
	{
		bool isFree = cb->GetImpl().isCpuFree && (cb->GetImpl().isGpuFree.getStatus() == vk::Result::eSuccess);

		if (isFree)
		{
			cb->GetImpl().isCpuFree = false;
			return cb.get();
		}
	}

	auto cb    = std::make_unique<CommandBuffer>(*this);
	cb->GetImpl().isCpuFree = false;

	auto cbPtr = cb.get();

	m_pImpl->commandBuffers[cbPtr] = std::move(cb);
	return cbPtr;
}

void Device::ReleaseCommandBuffer(CommandBuffer*& commandBuffer)
{
	std::scoped_lock lock(m_pImpl->commandBufferMutex);

	if (m_pImpl->commandBuffers.contains(commandBuffer))
		Defer([commandBuffer]() { commandBuffer->GetImpl().isCpuFree = true; });

	commandBuffer = nullptr;
}


std::optional<SemaphoreId> Device::Submit(CommandBuffer* commandBuffer, const std::vector<SemaphoreId>& waitOps /* = {} */, std::optional<SemaphoreId> signalOp /* = std::nullopt */)
{
	auto waitDestinationStageMasks = waitOps | std::views::transform([](const SemaphoreId&) {
		return vk::PipelineStageFlags(vk::PipelineStageFlagBits::eAllCommands);
	}) | std::ranges::to<std::vector>();

	const auto submitInfo = vk::SubmitInfo {
		   .waitSemaphoreCount   = static_cast<uint32_t>(waitOps.size()),
		   .pWaitSemaphores      = (vk::Semaphore*)waitOps.data(),
		   .pWaitDstStageMask    = waitDestinationStageMasks.data(),
		   .commandBufferCount   = 1,
		   .pCommandBuffers      = &*commandBuffer->GetImpl().commandBuffer,
		   .signalSemaphoreCount = 1,
		   .pSignalSemaphores    = signalOp ? (vk::Semaphore*)(&*signalOp) : &*commandBuffer->GetImpl().semaphore,
	};

	m_pImpl->device.resetFences({ commandBuffer->GetImpl().isGpuFree });
	m_pImpl->mainQueue.submit(submitInfo, commandBuffer->GetImpl().isGpuFree);

	if (signalOp)
		return std::nullopt;

	return s_FromVk(*commandBuffer->GetImpl().semaphore);
}

void Device::SubmitAndWait(CommandBuffer* commandBuffer, const std::vector<SemaphoreId>& waitOps /* = {} */) const
{
	auto waitDestinationStageMasks = waitOps | std::views::transform([](const SemaphoreId&) {
		return vk::PipelineStageFlags(vk::PipelineStageFlagBits::eAllCommands);
	}) | std::ranges::to<std::vector>();

	const auto submitInfo = vk::SubmitInfo {
		   .waitSemaphoreCount   = static_cast<uint32_t>(waitOps.size()),
		   .pWaitSemaphores      = (vk::Semaphore*)waitOps.data(),
		   .pWaitDstStageMask    = waitDestinationStageMasks.data(),
		   .commandBufferCount   = 1,
		   .pCommandBuffers      = &*commandBuffer->GetImpl().commandBuffer,
		   .signalSemaphoreCount = 0,
		   .pSignalSemaphores    = nullptr,
	};

	m_pImpl->device.resetFences({ commandBuffer->GetImpl().isGpuFree });
	m_pImpl->mainQueue.submit(submitInfo, commandBuffer->GetImpl().isGpuFree);
	if (m_pImpl->device.waitForFences({ commandBuffer->GetImpl().isGpuFree }, true, UINT64_MAX) != vk::Result::eSuccess)
		throw std::runtime_error("Error while waiting for fence");
}

void Device::WaitIdle()
{
	m_pImpl->device.waitIdle();

	// Process all pending deferred operations since we're idle now
	for (auto& fd : m_pImpl->frameData)
		for (auto& action : fd.deferredActions)
			action();
}

void Device::Defer(std::move_only_function<void()> action)
{
	m_pImpl->frameData[m_pImpl->currentFrame].deferredActions.push_back(std::move(action));
}

Device::AcquiredImage Device::AcquireNextImage()
{
	while (m_pImpl->device.waitForFences({ m_pImpl->frameData[m_pImpl->currentFrame].inFlightFence }, true, UINT64_MAX) != vk::Result::eSuccess) {}
	m_pImpl->device.resetFences({ m_pImpl->frameData[m_pImpl->currentFrame].inFlightFence });

	if (m_pImpl->swapchainNeedsRecreation)
		m_pImpl->RecreateSwapchain(*this, m_pImpl->window);

	for (auto& action : m_pImpl->frameData[m_pImpl->currentFrame].deferredActions)
		action();

	m_pImpl->frameData[m_pImpl->currentFrame].deferredActions.clear();

	vk::Result result;
	try
	{
		std::tie(result, m_pImpl->currentImageIndex) = m_pImpl->swapchain.acquireNextImage(UINT64_MAX, m_pImpl->frameData[m_pImpl->currentFrame].presentCompleteSemaphore, nullptr);
	}
	catch (const vk::OutOfDateKHRError&)
	{ // Surface is completely incompatible — recreate and retry once
		m_pImpl->RecreateSwapchain(*this, m_pImpl->window);
		std::tie(result, m_pImpl->currentImageIndex) = m_pImpl->swapchain.acquireNextImage( UINT64_MAX, m_pImpl->frameData[m_pImpl->currentFrame].presentCompleteSemaphore, nullptr);
	}

	// Valid but suboptimal image, recreate next frame
	if (result == vk::Result::eSuboptimalKHR)
		m_pImpl->swapchainNeedsRecreation = true;

	return AcquiredImage {
		.image           = *m_pImpl->swapchainImages[m_pImpl->currentImageIndex],
		.presentComplete = s_FromVk(*m_pImpl->frameData[m_pImpl->currentFrame].presentCompleteSemaphore),
		.renderComplete  = s_FromVk(*m_pImpl->frameData[m_pImpl->currentImageIndex].renderFinishedSemaphore)
	};
}

void Device::Present(const std::vector<SemaphoreId>& waitOps)
{
	auto vkWaitOps = waitOps | std::views::transform([](SemaphoreId s) { return vk::Semaphore(s_ToVk(s)); }) | std::ranges::to<std::vector>();

	const auto presentInfo = vk::PresentInfoKHR {
		.waitSemaphoreCount = static_cast<uint32_t>(vkWaitOps.size()),
		.pWaitSemaphores = vkWaitOps.data(),
		.swapchainCount  = 1,
		.pSwapchains     = &*m_pImpl->swapchain,
		.pImageIndices   = &m_pImpl->currentImageIndex
	};

	vk::Result presentResult;

	try
	{
		presentResult = m_pImpl->mainQueue.presentKHR(presentInfo);
	}
	catch (const vk::OutOfDateKHRError&)
	{
		presentResult = vk::Result::eErrorOutOfDateKHR;
	}

	if (presentResult == vk::Result::eSuboptimalKHR || presentResult == vk::Result::eErrorOutOfDateKHR)
		m_pImpl->swapchainNeedsRecreation = true;

	m_pImpl->mainQueue.submit({}, m_pImpl->frameData[m_pImpl->currentFrame].inFlightFence);
	++m_pImpl->currentFrame %= m_pImpl->frameData.size();
}
