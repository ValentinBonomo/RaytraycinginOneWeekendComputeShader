#ifndef GALLIUM__GPU__SHADERMODULE_REFLECTION_H
#define GALLIUM__GPU__SHADERMODULE_REFLECTION_H
#pragma once

#include <gallium/gpu/enums.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ga::gpu
{
    class  ShaderStructInstance;
    class  ShaderStructArrayInstance;
    struct ShaderStructDesc;

    using ShaderModuleReflectionData = std::unordered_map<std::string, std::shared_ptr<ShaderStructDesc>>;

    struct ShaderMemberDesc
    {
        std::string          name;
        uint32_t             offset      = 0;
        uint32_t             size        = 0;
        EShaderVariableType  type        = EShaderVariableType::Unknown;
        uint32_t             arraySize   = 0; // 0 = not an array
        uint32_t             arrayStride = 0;
        bool                 isPointer   = false;
        bool                 isMatrix    = false;

        // populated when type == Struct, isPointer == true, or array of structs
        std::shared_ptr<ShaderStructDesc> pointedDesc;
    };

    struct ShaderStructDesc
    {
        std::string                   name;
        uint32_t                      size = 0;
        std::vector<ShaderMemberDesc> members;

        const ShaderMemberDesc* Find(std::string_view memberName) const
        {
            for (auto& m : members)
                if (m.name == memberName)
                    return &m;

            return nullptr;
        }

        ShaderStructInstance      Instantiate() const;
        ShaderStructArrayInstance InstantiateArray(size_t count) const;
    };

    class ShaderMemberProxy
    {
        std::byte*       m_data;
        ShaderMemberDesc m_desc;

    public:
        ShaderMemberProxy(std::byte* data, const ShaderMemberDesc& desc)
            : m_data(data)
            , m_desc(desc) {}

        ShaderMemberProxy operator[](std::string_view name) const
        {
            const auto* m = m_desc.pointedDesc->Find(name);
            return ShaderMemberProxy(m_data + m->offset, *m);
        }

        ShaderMemberProxy operator[](size_t index) const
        {
            ShaderMemberDesc elementDesc = m_desc;
            elementDesc.arraySize   = 0;
            elementDesc.arrayStride = 0;
            return ShaderMemberProxy(m_data + m_desc.arrayStride * index, elementDesc);
        }

        template <typename T> ShaderMemberProxy& operator=(const T& v)      { assert(sizeof(T) <= m_desc.size && "ShaderMemberProxy: assignment overflows member size"); memcpy(m_data, &v, sizeof(T)); return *this; }
        template<>            ShaderMemberProxy& operator=(const double& v) { float f = float(v); memcpy(m_data, &f, sizeof(f)); return *this; }
        template<>            ShaderMemberProxy& operator=(const bool& v)   { uint32_t u = v; memcpy(m_data, &u, sizeof(u)); return *this; }
    };

    class ShaderStructInstance
    {
        std::shared_ptr<ShaderStructDesc> m_desc;
        std::vector<std::byte>            m_data;

    public:
        explicit ShaderStructInstance(std::shared_ptr<ShaderStructDesc> desc)
            : m_desc(std::move(desc))
            , m_data(m_desc->size, std::byte(0))
        {}

        ~ShaderStructInstance()
        {
            m_data.clear();
        }

        ShaderMemberProxy operator[](std::string_view name)
        {
            const auto* m = m_desc->Find(name);
            assert(m);
            return ShaderMemberProxy(m_data.data() + m->offset, *m);
        }

        const std::byte*        Data() const { return m_data.data(); }
        std::byte*              Data()       { return m_data.data(); }
        uint32_t                Size() const { return uint32_t(m_data.size()); }
        const ShaderStructDesc& Desc() const { return *m_desc; }
    };

    class ShaderStructArrayInstance
    {
        std::shared_ptr<ShaderStructDesc> m_elementDesc;
        size_t                            m_count;
        size_t                            m_stride;
        std::vector<std::byte>            m_data;

    public:
        ShaderStructArrayInstance(std::shared_ptr<ShaderStructDesc> desc, size_t count, size_t stride = 0)
            : m_elementDesc(std::move(desc))
            , m_count(count)
            , m_stride(stride != 0 ? stride : m_elementDesc->size)
            , m_data(m_stride * count, std::byte(0))
        {}

        ~ShaderStructArrayInstance()
        {
            m_data.clear();
        }

        ShaderMemberProxy operator[](size_t index)
        {
            assert(index < m_count);

            ShaderMemberDesc elemDesc;
            elemDesc.name        = m_elementDesc->name;
            elemDesc.type        = EShaderVariableType::Struct;
            elemDesc.size        = m_elementDesc->size;
            elemDesc.pointedDesc = m_elementDesc;

            return ShaderMemberProxy(m_data.data() + m_stride * index, elemDesc);
        }

        const std::byte* Data()   const { return m_data.data(); }
        std::byte*       Data()         { return m_data.data(); }
        size_t           Size()   const { return m_data.size(); }
        size_t           Count()  const { return m_count; }
        size_t           Stride() const { return m_stride; }

        const ShaderStructDesc& Desc() const { return *m_elementDesc; }
    };

    inline ShaderStructInstance ShaderStructDesc::Instantiate() const
    {
        return ShaderStructInstance(std::make_shared<ShaderStructDesc>(*this));
    }

    inline ShaderStructArrayInstance ShaderStructDesc::InstantiateArray(size_t count) const
    {
        return ShaderStructArrayInstance(std::make_shared<ShaderStructDesc>(*this), count);
    }
}

#endif /* GALLIUM__GPU__SHADERMODULE_REFLECTION_H */
