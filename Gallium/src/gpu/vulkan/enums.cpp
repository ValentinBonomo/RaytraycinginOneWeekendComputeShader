#include "enums_impl.h"

using namespace ga::gpu;

vk::BufferUsageFlags s_ToVk(EBufferUsage usage)
{
    vk::BufferUsageFlags result {};
    for (size_t i = 0; i < 32; ++i)
    {
        switch (usage & EBufferUsage(1 << i))
        {
        case EBufferUsage::UniformBuffer:                      result = result | vk::BufferUsageFlagBits::eUniformBuffer;                              break;
        case EBufferUsage::StorageBuffer:                      result = result | vk::BufferUsageFlagBits::eStorageBuffer;                              break;
        case EBufferUsage::VertexBuffer:                       result = result | vk::BufferUsageFlagBits::eVertexBuffer;                               break;
        case EBufferUsage::IndexBuffer:                        result = result | vk::BufferUsageFlagBits::eIndexBuffer;                                break;
        case EBufferUsage::TransferSrc:                        result = result | vk::BufferUsageFlagBits::eTransferSrc;                                break;
        // case EBufferUsage::ShaderDeviceAddress:                result = result | vk::BufferUsageFlagBits::eShaderDeviceAddress;                        break;
        case EBufferUsage::AccelerationStructureStorage:       result = result | vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;            break;
        case EBufferUsage::AccelerationStructureBuildReadonly: result = result | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR; break;
        default:
            break;
        }
    }

	return result;
}

vk::ImageLayout s_ToVk(EImageLayout layout)
{
	switch (layout)
	{
	case EImageLayout::General:								  return vk::ImageLayout::eGeneral;
	case EImageLayout::ColorAttachmentOptimal:				  return vk::ImageLayout::eColorAttachmentOptimal;
	case EImageLayout::DepthStencilAttachmentOptimal:		  return vk::ImageLayout::eDepthStencilAttachmentOptimal;
	case EImageLayout::DepthStencilReadOnlyOptimal:			  return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
	case EImageLayout::ShaderReadOnlyOptimal:				  return vk::ImageLayout::eShaderReadOnlyOptimal;
	case EImageLayout::TransferSrcOptimal:					  return vk::ImageLayout::eTransferSrcOptimal;
	case EImageLayout::TransferDstOptimal:					  return vk::ImageLayout::eTransferDstOptimal;
	case EImageLayout::Preinitialized:						  return vk::ImageLayout::ePreinitialized;
	case EImageLayout::DepthReadOnlyStencilAttachmentOptimal: return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
	case EImageLayout::DepthAttachmentStencilReadOnlyOptimal: return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
	case EImageLayout::DepthAttachmentOptimal:				  return vk::ImageLayout::eDepthAttachmentOptimal;
	case EImageLayout::DepthReadOnlyOptimal:				  return vk::ImageLayout::eDepthReadOnlyOptimal;
	case EImageLayout::StencilAttachmentOptimal:			  return vk::ImageLayout::eStencilAttachmentOptimal;
	case EImageLayout::StencilReadOnlyOptimal:				  return vk::ImageLayout::eStencilReadOnlyOptimal;
	case EImageLayout::ReadOnlyOptimal:						  return vk::ImageLayout::eReadOnlyOptimal;
	case EImageLayout::AttachmentOptimal:					  return vk::ImageLayout::eAttachmentOptimal;
	case EImageLayout::RenderingLocalRead:					  return vk::ImageLayout::eRenderingLocalRead;
	case EImageLayout::PresentSrc:							  return vk::ImageLayout::ePresentSrcKHR;
	default:
		break;
	}

	return vk::ImageLayout::eUndefined;
}

vk::ImageType s_ToVk(EImageType type)
{
	switch (type)
	{
	case EImageType::Image1D:
		return vk::ImageType::e1D;

	case EImageType::Image2D:
	case EImageType::ImageCube:
		return vk::ImageType::e2D;

	case EImageType::Image3D:
		return vk::ImageType::e3D;

	default:
		break;
	}
	return vk::ImageType(-1);
}

vk::ImageUsageFlags s_ToVk(EImageUsage usage)
{
	auto result = vk::ImageUsageFlags(0);

	if ((usage & EImageUsage::TransferSrc)     != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eTransferSrc;
	if ((usage & EImageUsage::TransferDst)     != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eTransferDst;
	if ((usage & EImageUsage::Sampled)         != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eSampled;
	if ((usage & EImageUsage::Storage)         != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eStorage;
	if ((usage & EImageUsage::ColorAttachment) != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eColorAttachment;
	if ((usage & EImageUsage::DepthStencil)    != EImageUsage::None) result = result | vk::ImageUsageFlagBits::eDepthStencilAttachment;

	return result;
}

vk::AttachmentLoadOp s_ToVk(ELoadOp op)
{
	switch (op)
	{
	case ELoadOp::Load:     return vk::AttachmentLoadOp::eLoad;
	case ELoadOp::Clear:    return vk::AttachmentLoadOp::eClear;
	case ELoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
	case ELoadOp::None:     return vk::AttachmentLoadOp::eNone;
	}

	return vk::AttachmentLoadOp(-1);
}

vk::AttachmentStoreOp s_ToVk(EStoreOp op)
{
	switch (op)
	{
	case EStoreOp::Store:    return vk::AttachmentStoreOp::eStore;
	case EStoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
	case EStoreOp::None:     return vk::AttachmentStoreOp::eNone;
	}

	return vk::AttachmentStoreOp(-1);
}

vk::ShaderStageFlagBits s_ToVk(EShaderStage stage)
{
	switch (stage)
	{
	case EShaderStage::Vertex:   return vk::ShaderStageFlagBits::eVertex;
	case EShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
    case EShaderStage::Compute:  return vk::ShaderStageFlagBits::eCompute;
	default:
		break;
	}
	return vk::ShaderStageFlagBits(-1);
}

EShaderStage s_FromVk(vk::ShaderStageFlagBits stage)
{
	switch (stage)
	{
	case vk::ShaderStageFlagBits::eVertex:   return EShaderStage::Vertex;
	case vk::ShaderStageFlagBits::eFragment: return EShaderStage::Fragment;
	default:
		break;
	}
	return EShaderStage(-1);
}

vk::Format s_ToVk(EFormat format)
{
	return vk::Format(format);
}

EFormat s_FromVk(vk::Format format)
{
	return EFormat(format);
}

size_t s_ToVkComponentSize(vk::Format value)
{
    switch (value)
    {
    case vk::Format::eR8Unorm:
    case vk::Format::eR8Snorm:
    case vk::Format::eR8Uint:
    case vk::Format::eR8Sint:
    case vk::Format::eR8Uscaled:
    case vk::Format::eR8Sscaled:
    case vk::Format::eR8G8Unorm:
    case vk::Format::eR8G8Snorm:
    case vk::Format::eR8G8Uint:
    case vk::Format::eR8G8Sint:
    case vk::Format::eR8G8Uscaled:
    case vk::Format::eR8G8Sscaled:
    case vk::Format::eR8G8B8Unorm:
    case vk::Format::eR8G8B8Snorm:
    case vk::Format::eR8G8B8Uint:
    case vk::Format::eR8G8B8Sint:
    case vk::Format::eR8G8B8Uscaled:
    case vk::Format::eR8G8B8Sscaled:
    case vk::Format::eR8G8B8Srgb:
    case vk::Format::eB8G8R8Unorm:
    case vk::Format::eB8G8R8Snorm:
    case vk::Format::eB8G8R8Uint:
    case vk::Format::eB8G8R8Sint:
    case vk::Format::eB8G8R8Uscaled:
    case vk::Format::eB8G8R8Sscaled:
    case vk::Format::eB8G8R8Srgb:
    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eR8G8B8A8Snorm:
    case vk::Format::eR8G8B8A8Uint:
    case vk::Format::eR8G8B8A8Sint:
    case vk::Format::eR8G8B8A8Uscaled:
    case vk::Format::eR8G8B8A8Sscaled:
    case vk::Format::eR8G8B8A8Srgb:
    case vk::Format::eB8G8R8A8Unorm:
    case vk::Format::eB8G8R8A8Snorm:
    case vk::Format::eB8G8R8A8Uint:
    case vk::Format::eB8G8R8A8Sint:
    case vk::Format::eB8G8R8A8Uscaled:
    case vk::Format::eB8G8R8A8Sscaled:
    case vk::Format::eB8G8R8A8Srgb:
    case vk::Format::eA8B8G8R8UnormPack32:
    case vk::Format::eA8B8G8R8SnormPack32:
    case vk::Format::eA8B8G8R8UintPack32:
    case vk::Format::eA8B8G8R8SintPack32:
    case vk::Format::eA8B8G8R8UscaledPack32:
    case vk::Format::eA8B8G8R8SscaledPack32:
    case vk::Format::eA8B8G8R8SrgbPack32:
        return 1;

    case vk::Format::eR16Unorm:
    case vk::Format::eR16Snorm:
    case vk::Format::eR16Uint:
    case vk::Format::eR16Sint:
    case vk::Format::eR16Uscaled:
    case vk::Format::eR16Sscaled:
    case vk::Format::eR16Sfloat:
    case vk::Format::eR16G16Unorm:
    case vk::Format::eR16G16Snorm:
    case vk::Format::eR16G16Uint:
    case vk::Format::eR16G16Sint:
    case vk::Format::eR16G16Uscaled:
    case vk::Format::eR16G16Sscaled:
    case vk::Format::eR16G16Sfloat:
    case vk::Format::eR16G16B16Unorm:
    case vk::Format::eR16G16B16Snorm:
    case vk::Format::eR16G16B16Uint:
    case vk::Format::eR16G16B16Sint:
    case vk::Format::eR16G16B16Uscaled:
    case vk::Format::eR16G16B16Sscaled:
    case vk::Format::eR16G16B16Sfloat:
    case vk::Format::eR16G16B16A16Unorm:
    case vk::Format::eR16G16B16A16Snorm:
    case vk::Format::eR16G16B16A16Uint:
    case vk::Format::eR16G16B16A16Sint:
    case vk::Format::eR16G16B16A16Uscaled:
    case vk::Format::eR16G16B16A16Sscaled:
    case vk::Format::eR16G16B16A16Sfloat:
        return 2;

    case vk::Format::eR32Uint:
    case vk::Format::eR32Sint:
    case vk::Format::eR32Sfloat:
    case vk::Format::eR32G32Uint:
    case vk::Format::eR32G32Sint:
    case vk::Format::eR32G32Sfloat:
    case vk::Format::eR32G32B32Uint:
    case vk::Format::eR32G32B32Sint:
    case vk::Format::eR32G32B32Sfloat:
    case vk::Format::eR32G32B32A32Uint:
    case vk::Format::eR32G32B32A32Sint:
    case vk::Format::eR32G32B32A32Sfloat:
        return 4;

    case vk::Format::eR64Sfloat:
    case vk::Format::eR64G64Sfloat:
    case vk::Format::eR64G64B64Sfloat:
    case vk::Format::eR64G64B64A64Sfloat:
        return 0;

    default:
        break;
    }

    return 0;
}

size_t s_ToVkComponentCount(vk::Format format)
{
    switch (format)
    {
    case vk::Format::eR8Unorm:
    case vk::Format::eR8Snorm:
    case vk::Format::eR8Uint:
    case vk::Format::eR8Sint:
    case vk::Format::eR8Uscaled:
    case vk::Format::eR8Sscaled:
    case vk::Format::eR16Unorm:
    case vk::Format::eR16Snorm:
    case vk::Format::eR16Uint:
    case vk::Format::eR16Sint:
    case vk::Format::eR16Uscaled:
    case vk::Format::eR16Sscaled:
    case vk::Format::eR16Sfloat:
    case vk::Format::eR32Uint:
    case vk::Format::eR32Sint:
    case vk::Format::eR32Sfloat:
    case vk::Format::eR64Sfloat:
        return 1;

    case vk::Format::eR8G8Unorm:
    case vk::Format::eR8G8Snorm:
    case vk::Format::eR8G8Uint:
    case vk::Format::eR8G8Sint:
    case vk::Format::eR8G8Uscaled:
    case vk::Format::eR8G8Sscaled:
    case vk::Format::eR16G16Unorm:
    case vk::Format::eR16G16Snorm:
    case vk::Format::eR16G16Uint:
    case vk::Format::eR16G16Sint:
    case vk::Format::eR16G16Uscaled:
    case vk::Format::eR16G16Sscaled:
    case vk::Format::eR16G16Sfloat:
    case vk::Format::eR32G32Uint:
    case vk::Format::eR32G32Sint:
    case vk::Format::eR32G32Sfloat:
    case vk::Format::eR64G64Sfloat:
        return 2;

    case vk::Format::eR8G8B8Unorm:
    case vk::Format::eR8G8B8Snorm:
    case vk::Format::eR8G8B8Uint:
    case vk::Format::eR8G8B8Sint:
    case vk::Format::eR8G8B8Uscaled:
    case vk::Format::eR8G8B8Sscaled:
    case vk::Format::eR8G8B8Srgb:
    case vk::Format::eR16G16B16Unorm:
    case vk::Format::eR16G16B16Snorm:
    case vk::Format::eR16G16B16Uint:
    case vk::Format::eR16G16B16Sint:
    case vk::Format::eR16G16B16Uscaled:
    case vk::Format::eR16G16B16Sscaled:
    case vk::Format::eR16G16B16Sfloat:
    case vk::Format::eR32G32B32Uint:
    case vk::Format::eR32G32B32Sint:
    case vk::Format::eR32G32B32Sfloat:
    case vk::Format::eR64G64B64Sfloat:
    case vk::Format::eR5G6B5UnormPack16:
    case vk::Format::eB8G8R8Unorm:
    case vk::Format::eB8G8R8Snorm:
    case vk::Format::eB8G8R8Uint:
    case vk::Format::eB8G8R8Sint:
    case vk::Format::eB8G8R8Uscaled:
    case vk::Format::eB8G8R8Sscaled:
    case vk::Format::eB8G8R8Srgb:
    case vk::Format::eB5G6R5UnormPack16:
        return 3;

    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eR8G8B8A8Snorm:
    case vk::Format::eR8G8B8A8Uint:
    case vk::Format::eR8G8B8A8Sint:
    case vk::Format::eR8G8B8A8Uscaled:
    case vk::Format::eR8G8B8A8Sscaled:
    case vk::Format::eR8G8B8A8Srgb:
    case vk::Format::eR16G16B16A16Unorm:
    case vk::Format::eR16G16B16A16Snorm:
    case vk::Format::eR16G16B16A16Uint:
    case vk::Format::eR16G16B16A16Sint:
    case vk::Format::eR16G16B16A16Uscaled:
    case vk::Format::eR16G16B16A16Sscaled:
    case vk::Format::eR16G16B16A16Sfloat:
    case vk::Format::eR32G32B32A32Uint:
    case vk::Format::eR32G32B32A32Sint:
    case vk::Format::eR32G32B32A32Sfloat:
    case vk::Format::eR64G64B64A64Sfloat:
    case vk::Format::eR4G4B4A4UnormPack16:
    case vk::Format::eR5G5B5A1UnormPack16:
    case vk::Format::eB8G8R8A8Unorm:
    case vk::Format::eB8G8R8A8Snorm:
    case vk::Format::eB8G8R8A8Uint:
    case vk::Format::eB8G8R8A8Sint:
    case vk::Format::eB8G8R8A8Uscaled:
    case vk::Format::eB8G8R8A8Sscaled:
    case vk::Format::eB8G8R8A8Srgb:
    case vk::Format::eB4G4R4A4UnormPack16:
    case vk::Format::eB5G5R5A1UnormPack16:
    case vk::Format::eA8B8G8R8UnormPack32:
    case vk::Format::eA8B8G8R8SnormPack32:
    case vk::Format::eA8B8G8R8UintPack32:
    case vk::Format::eA8B8G8R8SintPack32:
    case vk::Format::eA8B8G8R8UscaledPack32:
    case vk::Format::eA8B8G8R8SscaledPack32:
    case vk::Format::eA8B8G8R8SrgbPack32:
    case vk::Format::eA1R5G5B5UnormPack16:
        return 4;

    default:
        break;
    }

    return 0;
}

size_t s_ToVkPixelSize(vk::Format format)
{
    return s_ToVkComponentCount(format) * s_ToVkComponentSize(format);
}

vk::ImageAspectFlags s_DeduceVkAspectMask(EFormat format)
{
	switch (format)
	{
	case EFormat::D16_UNorm:
	case EFormat::D32_SFloat:
	case EFormat::X8_D24_UNormPack32:
		return vk::ImageAspectFlagBits::eDepth;

	case EFormat::S8_UInt:
		return vk::ImageAspectFlagBits::eStencil;

	case EFormat::D16_UNormS8_UInt:
	case EFormat::D24_UNormS8_UInt:
	case EFormat::D32_SFloatS8_UInt:
		return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

	default:
		break;
	}
	
	return vk::ImageAspectFlagBits::eColor;
}

vk::PipelineBindPoint s_ToVk(EPipelineClass bindPoint)
{
	switch (bindPoint)
	{
	case EPipelineClass::Graphics: return vk::PipelineBindPoint::eGraphics;
	case EPipelineClass::Compute:  return vk::PipelineBindPoint::eCompute;
	default:
		break;
	}
	return vk::PipelineBindPoint(-1);
}

vk::VertexInputRate s_ToVk(EVertexBindingRate rate)
{
	switch (rate)
	{
	case EVertexBindingRate::Vertex:   return vk::VertexInputRate::eVertex;
	case EVertexBindingRate::Instance: return vk::VertexInputRate::eInstance;
	}
	return vk::VertexInputRate(-1);
}

vk::PipelineStageFlags2 s_ToVk(EPipelineStage stage)
{
    vk::PipelineStageFlags2 flags = vk::PipelineStageFlagBits2::eNone;

    if ((stage & EPipelineStage::TopOfPipe) != EPipelineStage::None)             flags |= vk::PipelineStageFlagBits2::eTopOfPipe;
    if ((stage & EPipelineStage::DrawIndirect) != EPipelineStage::None)          flags |= vk::PipelineStageFlagBits2::eDrawIndirect;
    if ((stage & EPipelineStage::VertexInput) != EPipelineStage::None)           flags |= vk::PipelineStageFlagBits2::eVertexInput;
    if ((stage & EPipelineStage::VertexShader) != EPipelineStage::None)          flags |= vk::PipelineStageFlagBits2::eVertexShader;
    if ((stage & EPipelineStage::FragmentShader) != EPipelineStage::None)        flags |= vk::PipelineStageFlagBits2::eFragmentShader;
    if ((stage & EPipelineStage::EarlyFragmentTests) != EPipelineStage::None)    flags |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
    if ((stage & EPipelineStage::LateFragmentTests) != EPipelineStage::None)     flags |= vk::PipelineStageFlagBits2::eLateFragmentTests;
    if ((stage & EPipelineStage::ColorAttachmentOutput) != EPipelineStage::None) flags |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    if ((stage & EPipelineStage::ComputeShader) != EPipelineStage::None)         flags |= vk::PipelineStageFlagBits2::eComputeShader;
    if ((stage & EPipelineStage::Transfer) != EPipelineStage::None)              flags |= vk::PipelineStageFlagBits2::eTransfer;
    if ((stage & EPipelineStage::BottomOfPipe) != EPipelineStage::None)          flags |= vk::PipelineStageFlagBits2::eBottomOfPipe;
    if ((stage & EPipelineStage::Host) != EPipelineStage::None)                  flags |= vk::PipelineStageFlagBits2::eHost;
    if ((stage & EPipelineStage::AllGraphics) != EPipelineStage::None)           flags |= vk::PipelineStageFlagBits2::eAllGraphics;
    if ((stage & EPipelineStage::AllCommands) != EPipelineStage::None)           flags |= vk::PipelineStageFlagBits2::eAllCommands;

    return flags;
}

vk::AccessFlags2 s_ToVk(EAccessType access)
{
    vk::AccessFlags2 flags = vk::AccessFlagBits2::eNone;

    if ((access & EAccessType::IndirectCommandRead) != EAccessType::None)         flags |= vk::AccessFlagBits2::eIndirectCommandRead;
    if ((access & EAccessType::IndexRead) != EAccessType::None)                   flags |= vk::AccessFlagBits2::eIndexRead;
    if ((access & EAccessType::VertexAttributeRead) != EAccessType::None)         flags |= vk::AccessFlagBits2::eVertexAttributeRead;
    if ((access & EAccessType::UniformRead) != EAccessType::None)                 flags |= vk::AccessFlagBits2::eUniformRead;
    if ((access & EAccessType::ShaderRead) != EAccessType::None)                  flags |= vk::AccessFlagBits2::eShaderRead;
    if ((access & EAccessType::ShaderWrite) != EAccessType::None)                 flags |= vk::AccessFlagBits2::eShaderWrite;
    if ((access & EAccessType::ColorAttachmentRead) != EAccessType::None)         flags |= vk::AccessFlagBits2::eColorAttachmentRead;
    if ((access & EAccessType::ColorAttachmentWrite) != EAccessType::None)        flags |= vk::AccessFlagBits2::eColorAttachmentWrite;
    if ((access & EAccessType::DepthStencilAttachmentRead) != EAccessType::None)  flags |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
    if ((access & EAccessType::DepthStencilAttachmentWrite) != EAccessType::None) flags |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
    if ((access & EAccessType::TransferRead) != EAccessType::None)                flags |= vk::AccessFlagBits2::eTransferRead;
    if ((access & EAccessType::TransferWrite) != EAccessType::None)               flags |= vk::AccessFlagBits2::eTransferWrite;
    if ((access & EAccessType::HostRead) != EAccessType::None)                    flags |= vk::AccessFlagBits2::eHostRead;
    if ((access & EAccessType::HostWrite) != EAccessType::None)                   flags |= vk::AccessFlagBits2::eHostWrite;
    if ((access & EAccessType::MemoryRead) != EAccessType::None)                  flags |= vk::AccessFlagBits2::eMemoryRead;
    if ((access & EAccessType::MemoryWrite) != EAccessType::None)                 flags |= vk::AccessFlagBits2::eMemoryWrite;

    return flags;
}

vk::BlendFactor s_ToVk(ga::gpu::EBlendFactor f)
{
    return vk::BlendFactor(f);
}

vk::BlendOp s_ToVk(ga::gpu::EBlendOp o)
{
    return vk::BlendOp(o);
}

EBufferUsage operator|(EBufferUsage a, EBufferUsage b)
{
    return EBufferUsage(uint32_t(a) | uint32_t(b));
}

EBufferUsage operator&(EBufferUsage a, EBufferUsage b)
{
    return EBufferUsage(uint32_t(a) & uint32_t(b));
}

ECommandBufferUsage operator|(ECommandBufferUsage a, ECommandBufferUsage b)
{
    return ECommandBufferUsage(uint32_t(a) | uint32_t(b));
}

ECommandBufferUsage operator&(ECommandBufferUsage a, ECommandBufferUsage b)
{
    return ECommandBufferUsage(uint32_t(a) & uint32_t(b));
}

EImageUsage operator|(EImageUsage a, EImageUsage b)
{
	return EImageUsage(uint32_t(a) | uint32_t(b));
}

EImageUsage operator&(EImageUsage a, EImageUsage b)
{
	return EImageUsage(uint32_t(a) & uint32_t(b));
}

EShaderStage operator|(EShaderStage a, EShaderStage b)
{
    return EShaderStage(uint32_t(a) | uint32_t(b));
}

EShaderStage operator&(EShaderStage a, EShaderStage b)
{
    return EShaderStage(uint32_t(a) & uint32_t(b));
}

EPipelineStage operator|(EPipelineStage a, EPipelineStage b)
{
    return EPipelineStage(uint32_t(a) | uint32_t(b));
}

EPipelineStage operator&(EPipelineStage a, EPipelineStage b)
{
    return EPipelineStage(uint32_t(a) & uint32_t(b));
}

EAccessType operator|(EAccessType a, EAccessType b)
{
    return EAccessType(uint32_t(a) | uint32_t(b));
}

EAccessType operator&(EAccessType a, EAccessType b)
{
    return EAccessType(uint32_t(a) & uint32_t(b));
}
