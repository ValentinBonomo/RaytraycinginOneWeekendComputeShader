#ifndef GALLIUM__PLATFORM__VFS_H
#define GALLIUM__PLATFORM__VFS_H
#pragma once

#include <string>
#include <expected>
#include <functional>
#include <mutex>

namespace ga::platform
{
    enum class EVfsErrorCode
    {
        NotFound,
        NotAFile,
        NotADirectory,
        MountConflict,
        PermissionDenied,
        ProviderError
    };

    struct VfsError
    {
        using value_type = VfsError;

        EVfsErrorCode code;
        std::string   message;
    };

    template<typename T>
    using VfsResult = std::expected<T, VfsError>;

    enum class EVfsEntryKind
    {
        File,
        Directory
    };

    struct VfsEntry
    {
        std::string   name;
        EVfsEntryKind kind;
        std::size_t   size = 0;
    };

    struct VfsFileDesc
    {
        std::function<std::size_t(void* dst, std::size_t count)> read;
        std::function<void(std::size_t offset)>                  seek;
        std::function<std::size_t()>                             tell;
        std::function<std::size_t()>                             size;
        std::function<bool()>                                    eof;
    };

    struct VfsProviderDesc
    {
        std::function<bool(std::string_view path)>                                  exists;
        std::function<VfsResult<VfsFileDesc>(std::string_view path)>                open;
        std::function<VfsResult<std::vector<VfsEntry>>(std::string_view directory)> list;
    };

    class VfsFile
    {
        VfsFileDesc m_desc;

    public:
        explicit VfsFile(VfsFileDesc desc);

        VfsFile(const VfsFile&)            = delete;
        VfsFile& operator=(const VfsFile&) = delete;
        VfsFile(VfsFile&&)                 = default;
        VfsFile& operator=(VfsFile&&)      = default;

        std::size_t Read(void* dst, std::size_t count);
        void        Seek(std::size_t offset);

        [[nodiscard]] std::size_t Tell() const;
        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] bool        Eof() const;

        VfsResult<std::vector<std::byte>> ReadBytes();
        VfsResult<std::string>            ReadText();
    };

    class Vfs
    {
        struct MountPoint
        {
            std::string     path;
            VfsProviderDesc provider;
        };

        std::vector<MountPoint> m_mounts;
        mutable std::mutex      m_mutex;

        [[nodiscard]] std::string       m_StripPrefix(std::string_view mountPath, std::string_view fullPath) const;
        [[nodiscard]] const MountPoint* m_Resolve(std::string_view path) const;

    public:
        Vfs();
        ~Vfs()                     = default;
        Vfs(const Vfs&)            = delete;
        Vfs& operator=(const Vfs&) = delete;


        VfsResult<void> Mount(std::string_view mountPath, VfsProviderDesc provider);
        VfsResult<void> Unmount(std::string_view mountPath);

        VfsResult<VfsFile>               Open(std::string_view path)      const;
        VfsResult<std::vector<VfsEntry>> List(std::string_view directory) const;
        [[nodiscard]] bool               Exists(std::string_view path)    const;
    };
}

#endif /* GALLIUM__PLATFORM__VFS_H */
