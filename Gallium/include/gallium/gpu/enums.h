#ifndef GALLIUM__GPU__ENUMS_H
#define GALLIUM__GPU__ENUMS_H
#pragma once

#include <cstdint>

namespace ga::gpu
{
    enum class EBufferUsage
        : uint32_t
    {
        UniformBuffer                      = 1 << 0,
        StorageBuffer                      = 1 << 1,
        VertexBuffer                       = 1 << 2,
        IndexBuffer                        = 1 << 3,
        TransferSrc                        = 1 << 4,
        ShaderDeviceAddress                = 1 << 5,
        AccelerationStructureStorage       = 1 << 6,
        AccelerationStructureBuildReadonly = 1 << 7
    };

    enum class EImageLayout
    {
        Undefined,
        General,
        ColorAttachmentOptimal,
        DepthStencilAttachmentOptimal,
        DepthStencilReadOnlyOptimal,
        ShaderReadOnlyOptimal,
        TransferSrcOptimal,
        TransferDstOptimal,
        Preinitialized,
        DepthReadOnlyStencilAttachmentOptimal,
        DepthAttachmentStencilReadOnlyOptimal,
        DepthAttachmentOptimal,
        DepthReadOnlyOptimal,
        StencilAttachmentOptimal,
        StencilReadOnlyOptimal,
        ReadOnlyOptimal,
        AttachmentOptimal,
        RenderingLocalRead,
        PresentSrc
    };

    enum class EImageUsage
        : uint32_t
    {
        None            = 0,
        TransferSrc     = 1 << 0,
        TransferDst     = 1 << 1,
        Sampled         = 1 << 2,
        Storage         = 1 << 3,
        ColorAttachment = 1 << 4,
        DepthStencil    = 1 << 5
    };

    enum class EFormat
    {
        Undefined = 0,
        R4G4_UNormPack8 = 1,
        R4G4B4A4_UNormPack16 = 2,
        B4G4R4A4_UNormPack16 = 3,
        R5G6B5_UNormPack16 = 4,
        B5G6R5_UNormPack16 = 5,
        R5G5B5A1_UNormPack16 = 6,
        B5G5R5A1_UNormPack16 = 7,
        A1R5G5B5_UNormPack16 = 8,
        R8_UNorm = 9,
        R8_SNorm = 10,
        R8_UScaled = 11,
        R8_SScaled = 12,
        R8_UInt = 13,
        R8_SInt = 14,
        R8_SRGB = 15,
        R8G8_UNorm = 16,
        R8G8_SNorm = 17,
        R8G8_UScaled = 18,
        R8G8_SScaled = 19,
        R8G8_UInt = 20,
        R8G8_SInt = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNorm = 23,
        R8G8B8_SNorm = 24,
        R8G8B8_UScaled = 25,
        R8G8B8_SScaled = 26,
        R8G8B8_UInt = 27,
        R8G8B8_SInt = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNorm = 30,
        B8G8R8_SNorm = 31,
        B8G8R8_UScaled = 32,
        B8G8R8_SScaled = 33,
        B8G8R8_UInt = 34,
        B8G8R8_SInt = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNorm = 37,
        R8G8B8A8_SNorm = 38,
        R8G8B8A8_UScaled = 39,
        R8G8B8A8_SScaled = 40,
        R8G8B8A8_UInt = 41,
        R8G8B8A8_SInt = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNorm = 44,
        B8G8R8A8_SNorm = 45,
        B8G8R8A8_UScaled = 46,
        B8G8R8A8_SScaled = 47,
        B8G8R8A8_UInt = 48,
        B8G8R8A8_SInt = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNormPack32 = 51,
        A8B8G8R8_SNormPack32 = 52,
        A8B8G8R8_UScaledPack32 = 53,
        A8B8G8R8_SScaledPack32 = 54,
        A8B8G8R8_UIntPack32 = 55,
        A8B8G8R8_SIntPack32 = 56,
        A8B8G8R8_SRGBPack32 = 57,
        A2R10G10B10_UNormPack32 = 58,
        A2R10G10B10_SNormPack32 = 59,
        A2R10G10B10_UScaledPack32 = 60,
        A2R10G10B10_SScaledPack32 = 61,
        A2R10G10B10_UIntPack32 = 62,
        A2R10G10B10_SIntPack32 = 63,
        A2B10G10R10_UNormPack32 = 64,
        A2B10G10R10_SNormPack32 = 65,
        A2B10G10R10_UScaledPack32 = 66,
        A2B10G10R10_SScaledPack32 = 67,
        A2B10G10R10_UIntPack32 = 68,
        A2B10G10R10_SIntPack32 = 69,
        R16_UNorm = 70,
        R16_SNorm = 71,
        R16_UScaled = 72,
        R16_SScaled = 73,
        R16_UInt = 74,
        R16_SInt = 75,
        R16_SFloat = 76,
        R16G16_UNorm = 77,
        R16G16_SNorm = 78,
        R16G16_UScaled = 79,
        R16G16_SScaled = 80,
        R16G16_UInt = 81,
        R16G16_SInt = 82,
        R16G16_SFloat = 83,
        R16G16B16_UNorm = 84,
        R16G16B16_SNorm = 85,
        R16G16B16_UScaled = 86,
        R16G16B16_SScaled = 87,
        R16G16B16_UInt = 88,
        R16G16B16_SInt = 89,
        R16G16B16_SFloat = 90,
        R16G16B16A16_UNorm = 91,
        R16G16B16A16_SNorm = 92,
        R16G16B16A16_UScaled = 93,
        R16G16B16A16_SScaled = 94,
        R16G16B16A16_UInt = 95,
        R16G16B16A16_SInt = 96,
        R16G16B16A16_SFloat = 97,
        R32_UInt = 98,
        R32_SInt = 99,
        R32_SFloat = 100,
        R32G32_UInt = 101,
        R32G32_SInt = 102,
        R32G32_SFloat = 103,
        R32G32B32_UInt = 104,
        R32G32B32_SInt = 105,
        R32G32B32_SFloat = 106,
        R32G32B32A32_UInt = 107,
        R32G32B32A32_SInt = 108,
        R32G32B32A32_SFloat = 109,
        R64_UInt = 110,
        R64_SInt = 111,
        R64_SFloat = 112,
        R64G64_UInt = 113,
        R64G64_SInt = 114,
        R64G64_SFloat = 115,
        R64G64B64_UInt = 116,
        R64G64B64_SInt = 117,
        R64G64B64_SFloat = 118,
        R64G64B64A64_UInt = 119,
        R64G64B64A64_SInt = 120,
        R64G64B64A64_SFloat = 121,
        B10G11R11_UFloatPack32 = 122,
        E5B9G9R9_UFloatPack32 = 123,
        D16_UNorm = 124,
        X8_D24_UNormPack32 = 125,
        D32_SFloat = 126,
        S8_UInt = 127,
        D16_UNormS8_UInt = 128,
        D24_UNormS8_UInt = 129,
        D32_SFloatS8_UInt = 130,
        BC1_RGB_UNormBlock = 131,
        BC1_RGB_SRGBBlock = 132,
        BC1_RGBA_UNormBlock = 133,
        BC1_RGBA_SRGBBlock = 134,
        BC2_UNormBlock = 135,
        BC2_SRGBBlock = 136,
        BC3_UNormBlock = 137,
        BC3_SRGBBlock = 138,
        BC4_UNormBlock = 139,
        BC4_SNormBlock = 140,
        BC5_UNormBlock = 141,
        BC5_SNormBlock = 142,
        BC6H_UFloatBlock = 143,
        BC6H_SFloatBlock = 144,
        BC7_UNormBlock = 145,
        BC7_SRGBBlock = 146,
        ETC2_R8G8B8_UNormBlock = 147,
        ETC2_R8G8B8_SRGBBlock = 148,
        ETC2_R8G8B8A1_UNormBlock = 149,
        ETC2_R8G8B8A1_SRGBBlock = 150,
        ETC2_R8G8B8A8_UNormBlock = 151,
        ETC2_R8G8B8A8_SRGBBlock = 152,
        ASTC_4x4_UNormBlock = 157,
        ASTC_4x4_SRGBBlock = 158,
        ASTC_5x4_UNormBlock = 159,
        ASTC_5x4_SRGBBlock = 160,
        ASTC_5x5_UNormBlock = 161,
        ASTC_5x5_SRGBBlock = 162,
        ASTC_6x5_UNormBlock = 163,
        ASTC_6x5_SRGBBlock = 164,
        ASTC_6x6_UNormBlock = 165,
        ASTC_6x6_SRGBBlock = 166,
        ASTC_8x5_UNormBlock = 167,
        ASTC_8x5_SRGBBlock = 168,
        ASTC_8x6_UNormBlock = 169,
        ASTC_8x6_SRGBBlock = 170,
        ASTC_8x8_UNormBlock = 171,
        ASTC_8x8_SRGBBlock = 172,
        ASTC_10x5_UNormBlock = 173,
        ASTC_10x5_SRGBBlock = 174,
        ASTC_10x6_UNormBlock = 175,
        ASTC_10x6_SRGBBlock = 176,
        ASTC_10x8_UNormBlock = 177,
        ASTC_10x8_SRGBBlock = 178,
        ASTC_10x10_UNormBlock = 179,
        ASTC_10x10_SRGBBlock = 180,
        ASTC_12x10_UNormBlock = 181,
        ASTC_12x10_SRGBBlock = 182,
        ASTC_12x12_UNormBlock = 183,
        ASTC_12x12_SRGBBlock = 184,
        ASTC_4x4_SFloatBlock = 1000066000,
        ASTC_5x4_SFloatBlock = 1000066001,
        ASTC_5x5_SFloatBlock = 1000066002,
        ASTC_6x5_SFloatBlock = 1000066003,
        ASTC_6x6_SFloatBlock = 1000066004,
        ASTC_8x5_SFloatBlock = 1000066005,
        ASTC_8x6_SFloatBlock = 1000066006,
        ASTC_8x8_SFloatBlock = 1000066007,
        ASTC_10x5_SFloatBlock = 1000066008,
        ASTC_10x6_SFloatBlock = 1000066009,
        ASTC_10x8_SFloatBlock = 1000066010,
        ASTC_10x10_SFloatBlock = 1000066011,
        ASTC_12x10_SFloatBlock = 1000066012,
        ASTC_12x12_SFloatBlock = 1000066013,
        A1B5G5R5_UNormPack16 = 1000470000,
        A8_UNorm = 1000470001,
    };

    enum class EImageType
    {
        Image1D,
        Image2D,
        Image3D,
        ImageCube,

        Undefined = -1
    };

    enum class ELoadOp
    {
        Load,
        Clear,
        DontCare,
        None
    };

    enum class EStoreOp
    {
        Store,
        DontCare,
        None
    };

    enum class EPipelineClass
    {
        Graphics,
        Compute
    };

    enum class EVertexBindingRate
    {
        Vertex,
        Instance
    };

    enum class EShaderStage
    {
        Vertex   = 1 << 0,
        Fragment = 1 << 1,
        Compute  = 1 << 2
    };

    enum class EPipelineStage
        : uint32_t
    {
        None                  = 0,
        TopOfPipe             = 1 << 0,
        DrawIndirect          = 1 << 1,
        VertexInput           = 1 << 2,
        VertexShader          = 1 << 3,
        FragmentShader        = 1 << 4,
        EarlyFragmentTests    = 1 << 5,
        LateFragmentTests     = 1 << 6,
        ColorAttachmentOutput = 1 << 7,
        ComputeShader         = 1 << 8,
        Transfer              = 1 << 9,
        BottomOfPipe          = 1 << 10,
        Host                  = 1 << 11,
        AllGraphics           = 1 << 12,
        AllCommands           = 1 << 13
    };

    enum class EAccessType
        : uint32_t
    {
        None                        = 0,
        IndirectCommandRead         = 1 << 0,
        IndexRead                   = 1 << 1,
        VertexAttributeRead         = 1 << 2,
        UniformRead                 = 1 << 3,
        ShaderRead                  = 1 << 4,
        ShaderWrite                 = 1 << 5,
        ColorAttachmentRead         = 1 << 6,
        ColorAttachmentWrite        = 1 << 7,
        DepthStencilAttachmentRead  = 1 << 8,
        DepthStencilAttachmentWrite = 1 << 9,
        TransferRead                = 1 << 10,
        TransferWrite               = 1 << 11,
        HostRead                    = 1 << 12,
        HostWrite                   = 1 << 13,
        MemoryRead                  = 1 << 14,
        MemoryWrite                 = 1 << 15
    };

    enum class EShaderVariableType
    {
        Bool,
        Bool2,
        Bool3,
        Bool4,
        Int,
        Int2,
        Int3,
        Int4,
        UnsignedInt,
        UnsignedInt2,
        UnsignedInt3,
        UnsignedInt4,
        Float,
        Float2,
        Float3,
        Float4,
        Mat2,
        Mat3,
        Mat4,
        Mat2x3,
        Mat2x4,
        Mat3x2,
        Mat3x4,
        Mat4x2,
        Mat4x3,
        Struct,
        Pointer,
        Unknown
    };

    enum class ECommandBufferUsage
    {
        None          = 0,
        OneTimeSubmit = 1 << 0
    };

    enum class EBlendFactor
        : uint32_t
    {
        Zero = 0,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };

    enum class EBlendOp
        : uint32_t
    {
        Add = 0,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };
}

ga::gpu::EBufferUsage        operator|(ga::gpu::EBufferUsage a,        ga::gpu::EBufferUsage b);
ga::gpu::EBufferUsage        operator&(ga::gpu::EBufferUsage a,        ga::gpu::EBufferUsage b);
ga::gpu::ECommandBufferUsage operator|(ga::gpu::ECommandBufferUsage a, ga::gpu::ECommandBufferUsage b);
ga::gpu::ECommandBufferUsage operator&(ga::gpu::ECommandBufferUsage a, ga::gpu::ECommandBufferUsage b);
ga::gpu::EImageUsage         operator|(ga::gpu::EImageUsage a,         ga::gpu::EImageUsage b);
ga::gpu::EImageUsage         operator&(ga::gpu::EImageUsage a,         ga::gpu::EImageUsage b);
ga::gpu::EShaderStage        operator|(ga::gpu::EShaderStage a,        ga::gpu::EShaderStage b);
ga::gpu::EShaderStage        operator&(ga::gpu::EShaderStage a,        ga::gpu::EShaderStage b);
ga::gpu::EPipelineStage      operator|(ga::gpu::EPipelineStage a,      ga::gpu::EPipelineStage b);
ga::gpu::EPipelineStage      operator&(ga::gpu::EPipelineStage a,      ga::gpu::EPipelineStage b);
ga::gpu::EAccessType         operator|(ga::gpu::EAccessType a,         ga::gpu::EAccessType b);
ga::gpu::EAccessType         operator&(ga::gpu::EAccessType a,         ga::gpu::EAccessType b);

#endif /* GALLIUM__GPU__ENUMS_H */
