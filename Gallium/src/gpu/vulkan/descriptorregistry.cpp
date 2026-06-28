#include "descriptorregistry_impl.h"

#include "buffer_impl.h"
#include "image_impl.h"

using namespace ga::gpu;

uint32_t DescriptorRegistry::Impl::AllocSlot(EDescriptorType type)
{
    auto allocate = [](std::queue<uint32_t>& free, uint32_t& next, uint32_t max)
    {
        if (!free.empty()) { auto s = free.front(); free.pop(); return s; }
        return next++;
    };

    switch (type)
    {
    case EDescriptorType::Texture: return allocate(freeTextures, nextTexture, kMaxBindlessTextures);
    case EDescriptorType::Buffer:  return allocate(freeBuffers, nextBuffer, kMaxBindlessBuffers);
    }
    return kInvalidDescriptor;
}

void DescriptorRegistry::Impl::FreeSlot(DescriptorIndex index)
{
    switch (index.type)
    {
    case EDescriptorType::Texture: freeTextures.push(index.slot); break;
    case EDescriptorType::Buffer:  freeBuffers.push(index.slot);  break;
    }
}

// ----------------------------------------------------------------------------

static constexpr uint32_t kBINDING_TEXTURES       = 0;
static constexpr uint32_t kBINDING_STORAGE_IMAGES = 1;
static constexpr uint32_t kBINDING_BUFFERS        = 2;

DescriptorRegistry::DescriptorRegistry(const Device& device)
    : m_pImpl(new Impl)
{
    const std::array poolSizes =
    {
        vk::DescriptorPoolSize { vk::DescriptorType::eCombinedImageSampler, kMaxBindlessTextures },
        vk::DescriptorPoolSize { vk::DescriptorType::eStorageImage,         kMaxBindlessTextures },
        vk::DescriptorPoolSize { vk::DescriptorType::eStorageBuffer,        kMaxBindlessBuffers  },
    };

    m_pImpl->pool = vk::raii::DescriptorPool(device.GetImpl().device, vk::DescriptorPoolCreateInfo{
        .flags         = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets       = 1,
        .poolSizeCount = uint32_t(poolSizes.size()),
        .pPoolSizes    = poolSizes.data(),
    });

    const std::array bindings =
    {
        vk::DescriptorSetLayoutBinding { kBINDING_TEXTURES,       vk::DescriptorType::eCombinedImageSampler, kMaxBindlessTextures, vk::ShaderStageFlagBits::eAll },
        vk::DescriptorSetLayoutBinding { kBINDING_STORAGE_IMAGES, vk::DescriptorType::eStorageImage,         kMaxBindlessTextures, vk::ShaderStageFlagBits::eAll },
        vk::DescriptorSetLayoutBinding { kBINDING_BUFFERS,        vk::DescriptorType::eStorageBuffer,        kMaxBindlessBuffers,  vk::ShaderStageFlagBits::eAll },
    };

    const std::array bindingFlags =
    {
        vk::DescriptorBindingFlags { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind },
        vk::DescriptorBindingFlags { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind },
        vk::DescriptorBindingFlags { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind },
    };

    const auto bindingFlagsInfo = vk::DescriptorSetLayoutBindingFlagsCreateInfo {
        .bindingCount  = uint32_t(bindingFlags.size()),
        .pBindingFlags = bindingFlags.data(),
    };

    m_pImpl->layout = vk::raii::DescriptorSetLayout(device.GetImpl().device, vk::DescriptorSetLayoutCreateInfo {
        .pNext        = &bindingFlagsInfo,
        .flags        = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
        .bindingCount = uint32_t(bindings.size()),
        .pBindings    = bindings.data(),
    });

    auto allocInfo = vk::DescriptorSetAllocateInfo {
        .descriptorPool     = *m_pImpl->pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &*m_pImpl->layout,
    };

    auto sets = vk::raii::DescriptorSets(device.GetImpl().device, allocInfo);
    m_pImpl->set = std::move(sets[0]);
}

DescriptorRegistry::~DescriptorRegistry()
{
    m_pImpl->set.clear();
    m_pImpl->layout.clear();
    m_pImpl->pool.clear();
}

const DescriptorRegistry::Impl& DescriptorRegistry::GetImpl() const
{
    return *m_pImpl;
}

DescriptorIndex DescriptorRegistry::Register(const Image& image)
{
    auto& impl = *m_pImpl;
    std::vector<vk::WriteDescriptorSet> writes;

    const auto index = DescriptorIndex {
        .slot = m_pImpl->AllocSlot(EDescriptorType::Texture),
        .type = EDescriptorType::Texture
    };

    const auto imageInfo = vk::DescriptorImageInfo {
        .sampler     = *image.GetImpl().sampler,
        .imageView   = *image.GetImpl().imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    };

    writes.push_back({
        .dstSet          = *m_pImpl->set,
        .dstBinding      = kBINDING_TEXTURES,
        .dstArrayElement = index.slot,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo      = &imageInfo,
    });

    const auto storageInfo = vk::DescriptorImageInfo {
        .sampler     = nullptr,
        .imageView   = *image.GetImpl().imageView,
        .imageLayout = vk::ImageLayout::eGeneral,
    };

    const bool isStorage = (image.GetImpl().usage & vk::ImageUsageFlagBits::eStorage) == vk::ImageUsageFlagBits::eStorage;
    if (isStorage)
    {
        writes.push_back({
            .dstSet          = *m_pImpl->set,
            .dstBinding      = kBINDING_STORAGE_IMAGES,
            .dstArrayElement = index.slot,
            .descriptorCount = 1,
            .descriptorType  = vk::DescriptorType::eStorageImage,
            .pImageInfo      = &storageInfo,
        });
    }

    m_pImpl->set.getDevice().updateDescriptorSets(writes, nullptr);
    return index;
}

DescriptorIndex DescriptorRegistry::Register(const Buffer& buffer)
{
    auto& impl = *m_pImpl;

    const auto index = DescriptorIndex {
        .slot = m_pImpl->AllocSlot(EDescriptorType::Buffer),
        .type = EDescriptorType::Buffer
    };

    const auto bufferInfo = vk::DescriptorBufferInfo {
        .buffer = *buffer.GetImpl().buffer,
        .offset = 0,
        .range  = vk::WholeSize,
    };

    const auto write = vk::WriteDescriptorSet {
        .dstSet          = *m_pImpl->set,
        .dstBinding      = kBINDING_BUFFERS,
        .dstArrayElement = index.slot,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo     = &bufferInfo,
    };

    m_pImpl->set.getDevice().updateDescriptorSets(write, nullptr);
    return index;
}

void DescriptorRegistry::Unregister(DescriptorIndex& index)
{
    if (!index.IsValid())
        return;

    m_pImpl->FreeSlot(index);
    index.slot = kInvalidDescriptor;
    index.type = EDescriptorType::Invalid;
}
