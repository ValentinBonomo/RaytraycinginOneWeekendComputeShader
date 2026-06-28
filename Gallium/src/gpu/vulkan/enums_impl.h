#pragma once

#include <gallium/gpu/enums.h>
#include "device_impl.h"

vk::BufferUsageFlags    s_ToVk(ga::gpu::EBufferUsage bufferClass);

vk::ImageLayout         s_ToVk(ga::gpu::EImageLayout layout);
vk::ImageType           s_ToVk(ga::gpu::EImageType type);
vk::ImageUsageFlags     s_ToVk(ga::gpu::EImageUsage usage);
vk::Format              s_ToVk(ga::gpu::EFormat format);
ga::gpu::EFormat        s_FromVk(vk::Format format);
vk::ImageAspectFlags    s_DeduceVkAspectMask(ga::gpu::EFormat format);

size_t                  s_ToVkComponentSize(vk::Format value);
size_t                  s_ToVkComponentCount(vk::Format format);
size_t                  s_ToVkPixelSize(vk::Format format);

vk::AttachmentLoadOp    s_ToVk(ga::gpu::ELoadOp op);
vk::AttachmentStoreOp   s_ToVk(ga::gpu::EStoreOp op);

vk::ShaderStageFlagBits s_ToVk(ga::gpu::EShaderStage stage);
ga::gpu::EShaderStage   s_FromVk(vk::ShaderStageFlagBits stage);
vk::PipelineBindPoint   s_ToVk(ga::gpu::EPipelineClass bindPoint);
vk::VertexInputRate     s_ToVk(ga::gpu::EVertexBindingRate rate);

vk::PipelineStageFlags2 s_ToVk(ga::gpu::EPipelineStage stage);
vk::AccessFlags2        s_ToVk(ga::gpu::EAccessType access);

vk::BlendFactor         s_ToVk(ga::gpu::EBlendFactor f);
vk::BlendOp             s_ToVk(ga::gpu::EBlendOp o);