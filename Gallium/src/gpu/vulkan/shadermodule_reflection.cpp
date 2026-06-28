#include "shadermodule_reflection_impl.h"

#ifndef NDEBUG
# pragma comment(lib, "SPIRV-reflect-d.lib")
#else
# pragma comment(lib, "SPIRV-reflect.lib")
#endif

using namespace ga::gpu;

static std::string CleanupTypeName(std::string_view name)
{
    std::string r(name);
    for (auto suffix : { "_natural", "_std430", "_std140" })
        if (r.ends_with(suffix)) { r.resize(r.size() - std::string_view(suffix).size()); break; }
    return r;
}

static std::string GuessTypeName(const SpvReflectTypeDescription& type)
{
    std::string base;
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_VOID)  base = "void";
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)  base = "bool";
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        base = type.traits.numeric.scalar.signedness ? "int" : "uint";
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) base = "float";
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
        base += std::to_string(type.traits.numeric.vector.component_count);
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
        base += std::to_string(type.traits.numeric.matrix.column_count)
        + "x"
        + std::to_string(type.traits.numeric.matrix.row_count);
    return base;
}

static bool DetectSlangMatrix(const SpvReflectBlockVariable& member, uint32_t& outCols, uint32_t& outRows, uint32_t& outCount)
{
    if (member.member_count != 1)                                               return false;
    if (!member.members[0].name || strcmp(member.members[0].name, "data") != 0) return false;

    const auto& data = member.members[0];
    if (data.array.dims_count == 0 || !data.type_description)                   return false;
    const auto dataFlags = data.type_description->type_flags;

    // Standalone matrix
    if ((dataFlags & SPV_REFLECT_TYPE_FLAG_FLOAT) && (dataFlags & SPV_REFLECT_TYPE_FLAG_VECTOR))
    {
        outCount = 1;
        outCols  = data.array.dims[0];
        outRows  = data.type_description->traits.numeric.vector.component_count;
        return true;
    }

    // Array of matrices
    if ((dataFlags & SPV_REFLECT_TYPE_FLAG_STRUCT) && (dataFlags & SPV_REFLECT_TYPE_FLAG_ARRAY))
    {
        if (data.member_count != 1)                                             return false;
        if (!data.members[0].name || strcmp(data.members[0].name, "data") != 0) return false;

        const auto& inner = data.members[0];
        const auto  innerFlags = inner.type_description->type_flags;
        if (inner.array.dims_count == 0 || !inner.type_description)             return false;
        if (!(innerFlags & SPV_REFLECT_TYPE_FLAG_FLOAT))                        return false;
        if (!(innerFlags & SPV_REFLECT_TYPE_FLAG_VECTOR))                       return false;

        outCount = data.array.dims[0];
        outCols  = inner.array.dims[0];
        outRows  = inner.type_description->traits.numeric.vector.component_count;
        return true;
    }

    return false;
}

static EShaderVariableType MatrixType(uint32_t cols, uint32_t rows)
{
    if (cols == rows) {
        switch (cols) {
        case 2: return EShaderVariableType::Mat2;
        case 3: return EShaderVariableType::Mat3;
        case 4: return EShaderVariableType::Mat4;
        }
    }
    if (rows == 2 && cols == 3) return EShaderVariableType::Mat2x3;
    if (rows == 2 && cols == 4) return EShaderVariableType::Mat2x4;
    if (rows == 3 && cols == 2) return EShaderVariableType::Mat3x2;
    if (rows == 3 && cols == 4) return EShaderVariableType::Mat3x4;
    if (rows == 4 && cols == 2) return EShaderVariableType::Mat4x2;
    if (rows == 4 && cols == 3) return EShaderVariableType::Mat4x3;
    return EShaderVariableType::Unknown;
}

EShaderVariableType ReflectSpvVariableType(const SpvReflectTypeDescription& type)
{
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_REF)    return EShaderVariableType::Pointer;
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) return EShaderVariableType::Struct;

    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
        return MatrixType(
            type.traits.numeric.matrix.column_count,
            type.traits.numeric.matrix.row_count
        );

    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
    {
        const uint32_t n = type.traits.numeric.vector.component_count;
        const bool     isSigned = type.traits.numeric.scalar.signedness != 0;
        if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) switch (n) {
        case 2: return EShaderVariableType::Float2;
        case 3: return EShaderVariableType::Float3;
        case 4: return EShaderVariableType::Float4;
        }
        if (type.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        {
            if (isSigned)  switch (n) {
            case 2: return EShaderVariableType::Int2;
            case 3: return EShaderVariableType::Int3;
            case 4: return EShaderVariableType::Int4;
            }
            else switch (n) {
            case 2: return EShaderVariableType::UnsignedInt2;
            case 3: return EShaderVariableType::UnsignedInt3;
            case 4: return EShaderVariableType::UnsignedInt4;
            }
        }
        if (type.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL) switch (n) {
        case 2: return EShaderVariableType::Bool2;
        case 3: return EShaderVariableType::Bool3;
        case 4: return EShaderVariableType::Bool4;
        }
    }

    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) return EShaderVariableType::Float;
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)  return EShaderVariableType::Bool;
    if (type.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        return type.traits.numeric.scalar.signedness
        ? EShaderVariableType::Int : EShaderVariableType::UnsignedInt;

    return EShaderVariableType::Unknown;
}

ShaderMemberDesc ReflectSpvBlockVariable(const SpvReflectBlockVariable& member)
{
    ShaderMemberDesc desc;
    desc.name   = member.name ? member.name : "";
    desc.offset = member.offset;
    desc.size   = member.size;

    assert(member.type_description);
    const auto& td = *member.type_description;

    const bool        isPointer = td.type_flags & SPV_REFLECT_TYPE_FLAG_REF;
    const bool        isStruct  = td.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT;
    const bool        isArray   = td.type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY;
    const std::string typeName  = td.type_name
        ? CleanupTypeName(td.type_name)
        : GuessTypeName(td);

    if (isPointer)
    {
        desc.isPointer = true;
        desc.type      = EShaderVariableType::Pointer;
        desc.size      = sizeof(uintptr_t);

        if (member.member_count > 0)
        {
            auto p  = std::make_shared<ShaderStructDesc>();
            p->name = typeName;
            p->size = member.members[member.member_count - 1].offset
                     + member.members[member.member_count - 1].size;
            for (uint32_t i = 0; i < member.member_count; ++i)
                p->members.push_back(ReflectSpvBlockVariable(member.members[i]));
            desc.pointedDesc = std::move(p);
        }
        return desc;
    }

    uint32_t mCols = 0, mRows = 0, mCount = 0;
    if (DetectSlangMatrix(member, mCols, mRows, mCount))
    {
        desc.type     = MatrixType(mCols, mRows);
        desc.isMatrix = true;
        if (mCount > 1) { desc.arraySize = mCount; desc.arrayStride = member.size / mCount; }
        return desc;
    }

    if (isArray)
    {
        desc.arraySize   = member.array.dims_count > 0 ? member.array.dims[0] : 0;
        desc.arrayStride = member.array.stride;

        if (td.op == SpvOpTypeRuntimeArray)
            { desc.arraySize = 0; desc.arrayStride = td.traits.array.stride; }

        // Array of structs
        if (isStruct && member.member_count > 0)
        {
            desc.type = EShaderVariableType::Struct;
            auto e  = std::make_shared<ShaderStructDesc>();
            e->name = typeName; e->size = desc.arrayStride;
            for (uint32_t i = 0; i < member.member_count; ++i)
                e->members.push_back(ReflectSpvBlockVariable(member.members[i]));
            desc.pointedDesc = std::move(e);
            return desc;
        }

        // Array of scalars / vectors
        SpvReflectTypeDescription et = td;
        et.type_flags &= ~SPV_REFLECT_TYPE_FLAG_ARRAY;
        desc.type = ReflectSpvVariableType(et);
        return desc;
    }

    // Nested structs
    if (isStruct)
    {
        desc.type = EShaderVariableType::Struct;
        auto n  = std::make_shared<ShaderStructDesc>();
        n->name = typeName; n->size = member.size;
        for (uint32_t i = 0; i < member.member_count; ++i)
            n->members.push_back(ReflectSpvBlockVariable(member.members[i]));
        desc.pointedDesc = std::move(n);
        return desc;
    }

    desc.type = ReflectSpvVariableType(td);
    return desc;
}

std::shared_ptr<ShaderStructDesc> ReflectSpvStruct(const SpvReflectBlockVariable& block, std::string_view name)
{
    auto desc  = std::make_shared<ShaderStructDesc>();
    desc->name = std::string(name);
    desc->size = block.size;
    for (uint32_t i = 0; i < block.member_count; ++i)
        desc->members.push_back(ReflectSpvBlockVariable(block.members[i]));
    return desc;
}

static void CollectSpvStructsInternal(const ShaderStructDesc& desc, std::unordered_map<std::string, std::shared_ptr<ShaderStructDesc>>& out)
{
    for (const auto& m : desc.members)
    {
        if (!m.pointedDesc || m.pointedDesc->name.empty())
            continue;

        if (out.try_emplace(m.pointedDesc->name, m.pointedDesc).second)
            CollectSpvStructsInternal(*m.pointedDesc, out);
    }
}


static std::vector<std::shared_ptr<ShaderStructDesc>> ReflectAllSpvStructs(const SpvReflectShaderModule& module)
{
    std::unordered_map<std::string, std::shared_ptr<ShaderStructDesc>> seen;

    auto insert = [&](std::shared_ptr<ShaderStructDesc> d)
        { if (d && !d->name.empty()) seen.try_emplace(d->name, std::move(d)); };

    // Push constant blocks
    for (uint32_t i = 0; i < module.push_constant_block_count; ++i)
    {
        const auto& block = module.push_constant_blocks[i];
        const std::string name = block.type_description->type_name
            ? CleanupTypeName(block.type_description->type_name) : "__UNNAMED__";
        insert(ReflectSpvStruct(block, name));
    }

    // Storage buffer descriptor bindings
    for (uint32_t i = 0; i < module.descriptor_set_count; ++i)
    {
        const auto& set = module.descriptor_sets[i];
        for (uint32_t j = 0; j < set.binding_count; ++j)
        {
            const auto& b = *set.bindings[j];
            if (b.descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) continue;
            if (b.block.member_count == 0)                                       continue;
            const char* tn = b.type_description ? b.type_description->type_name : b.name;
            if (!tn) continue;
            insert(ReflectSpvStruct(b.block, CleanupTypeName(tn)));
        }
    }

    std::vector<std::shared_ptr<ShaderStructDesc>> result;
    result.reserve(seen.size());
    for (auto& [_, d] : seen) result.push_back(std::move(d));
    return result;
}

static std::unordered_map<std::string, std::shared_ptr<ShaderStructDesc>> CollectSpvStructs(const std::vector<std::shared_ptr<ShaderStructDesc>>& roots)
{
    std::unordered_map<std::string, std::shared_ptr<ShaderStructDesc>> result;
    for (const auto& root : roots)
    {
        result.try_emplace(root->name, root);
        CollectSpvStructsInternal(*root, result);
    }
    return result;
}

ga::gpu::ShaderModuleReflectionData ga::gpu::ReflectSpvShaderModule(size_t size, const void* data)
{
    SpvReflectShaderModule reflectModule;
    spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, size, data, &reflectModule);
    return CollectSpvStructs(ReflectAllSpvStructs(reflectModule));
}
