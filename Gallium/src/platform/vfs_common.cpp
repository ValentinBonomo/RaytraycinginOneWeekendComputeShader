#include <gallium/platform/vfs.h>

#include <filesystem>
#include <fstream>
#include <print>
#include <cassert>

using namespace ga::platform;

namespace
{
    std::string NormalizePath(std::string_view path)
    {
        std::string out;
        out.reserve(path.size());
        bool lastSlash = false;
        for (char c : path)
        {
            if (c == '/')
            {
                if (!lastSlash) out += c;
                lastSlash = true;
            }
            else
            {
                out += c;
                lastSlash = false;
            }
        }

        if (out.size() > 1 && out.back() == '/')
            out.pop_back();
        return out;
    }

    std::string_view TopComponent(std::string_view path)
    {
        auto start = path.find_first_not_of('/');
        if (start == std::string_view::npos) return {};
        auto end = path.find('/', start);
        return path.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
    }

    VfsError OsError(EVfsErrorCode code, const std::filesystem::path& p, const std::error_code& ec)
    {
        return { code, p.string() + ": " + ec.message() };
    }

    VfsProviderDesc MakeDiskProvider(std::filesystem::path root)
    {
        return VfsProviderDesc
        {
            .exists = [root](std::string_view relPath) {
                std::error_code ec;
                return std::filesystem::exists(root / relPath, ec);
            },

            .open = [root](std::string_view relPath) -> VfsResult<VfsFileDesc>
            {
                std::filesystem::path full = root / relPath;
                std::error_code ec;

                if (!std::filesystem::exists(full, ec))
                    return std::unexpected(VfsError {
                        EVfsErrorCode::NotFound,
                        std::format("{}: not found", full.string())
                    });

                if (std::filesystem::is_directory(full, ec))
                    return std::unexpected(VfsError {
                        EVfsErrorCode::NotAFile,
                        std::format("{}: not a file", full.string())
                    });

                auto stream = std::make_shared<std::ifstream>(full, std::ios::binary);
                if (!stream->is_open())
                    return std::unexpected(VfsError {
                        EVfsErrorCode::PermissionDenied,
                        std::format("{}: could not open", full.string())
                    });

                stream->seekg(0, std::ios::end);
                const std::size_t fileSize = static_cast<std::size_t>(stream->tellg());
                stream->seekg(0, std::ios::beg);

                return VfsFileDesc
                {
                    .read = [stream](void* dst, std::size_t count) {
                        stream->read(static_cast<char*>(dst), static_cast<std::streamsize>(count));
                        return static_cast<std::size_t>(stream->gcount());
                    },
                    .seek = [stream](std::size_t offset) {
                        stream->clear();
                        stream->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
                    },
                    .tell = [stream]() {
                        return static_cast<std::size_t>(stream->tellg());
                    },
                    .size = [fileSize]() {
                        return fileSize;
                    },
                    .eof = [stream]() {
                        return stream->eof();
                    }
                };
            },

            .list = [root](std::string_view relPath) -> VfsResult<std::vector<VfsEntry>>
            {
                std::filesystem::path dir = relPath.empty() ? root : root / relPath;
                std::error_code ec;

                if (!std::filesystem::exists(dir, ec))
                    return std::unexpected(VfsError {
                        EVfsErrorCode::NotFound,
                        std::format("{}: not found", dir.string())
                    });

                if (!std::filesystem::is_directory(dir, ec))
                    return std::unexpected(VfsError {
                        EVfsErrorCode::NotADirectory,
                        std::format("{}: not a directory", dir.string())
                    });

                std::vector<VfsEntry> entries;
                for (const auto& dirent : std::filesystem::directory_iterator(dir, ec))
                {
                    if (ec)
                        break;

                    VfsEntry entry;
                    entry.name = dirent.path().filename().string();

                    if (dirent.is_directory(ec))
                    {
                        entry.kind = EVfsEntryKind::Directory;
                        entry.size = 0;
                    }
                    else
                    {
                        entry.kind = EVfsEntryKind::File;
                        entry.size = static_cast<std::size_t>(dirent.file_size(ec));
                        if (ec)
                            entry.size = 0; // Non-fatal; size unknown.
                    }

                    entries.push_back(std::move(entry));
                }

                if (ec)
                    return std::unexpected(OsError(EVfsErrorCode::ProviderError, dir, ec));

                return entries;
            },
        };
    }
}

// ----------------------------------------------------------------------------

VfsFile::VfsFile(VfsFileDesc desc)
    : m_desc(std::move(desc))
{
    assert(m_desc.read && "VfsFileDesc::read must not be nullptr");
    assert(m_desc.seek && "VfsFileDesc::seek must not be nullptr");
    assert(m_desc.tell && "VfsFileDesc::tell must not be nullptr");
    assert(m_desc.size && "VfsFileDesc::size must not be nullptr");
    assert(m_desc.eof  && "VfsFileDesc::eof  must not be nullptr");
}

std::size_t VfsFile::Read(void* dst, std::size_t count)
{
    return m_desc.read(dst, count);
}

void VfsFile::Seek(std::size_t offset)
{
    m_desc.seek(offset);
}

std::size_t VfsFile::Tell() const
{
    return m_desc.tell();
}

std::size_t VfsFile::Size() const
{
    return m_desc.size();
}

bool VfsFile::Eof() const
{
    return m_desc.eof();
}

VfsResult<std::vector<std::byte>> VfsFile::ReadBytes()
{
    const std::size_t fileSize = m_desc.size();
    m_desc.seek(0);

    std::vector<std::byte> buf(fileSize);
    std::size_t totalRead = 0;

    while (totalRead < fileSize && !m_desc.eof())
    {
        std::size_t n = m_desc.read(reinterpret_cast<char*>(buf.data()) + totalRead, fileSize - totalRead);
        if (n == 0)
            break; // Provider signaled stall; avoid spinning.
        totalRead += n;
    }

    if (totalRead != fileSize)
        return std::unexpected(VfsError{
            EVfsErrorCode::ProviderError,
            std::format("ReadBytes: expected {} bytes, got {}", fileSize, totalRead)
            });

    return buf;
}

VfsResult<std::string> VfsFile::ReadText()
{
    return ReadBytes().transform([](std::vector<std::byte>&& bytes) {
        return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        });
}

// -----------------------------------------------------------------------------

std::string Vfs::m_StripPrefix(std::string_view mountPath, std::string_view fullPath) const
{
    if (mountPath == "/")
    {
        if (fullPath.size() <= 1) return {};
        return std::string(fullPath.substr(1));
    }

    if (fullPath.size() <= mountPath.size())
        return {};

    return std::string(fullPath.substr(mountPath.size() + 1));
}

const Vfs::MountPoint* Vfs::m_Resolve(std::string_view path) const
{
    for (const auto& mp : m_mounts)
    {
        if (mp.path == "/")
            return &mp;

        if (path.starts_with(mp.path))
            if (path.size() == mp.path.size() || path[mp.path.size()] == '/')
                return &mp;
    }

    return nullptr;
}

// -----------------------------------------------------------------------------

Vfs::Vfs()
{
    m_mounts.push_back(MountPoint {
        .path = "/",
        .provider = MakeDiskProvider(std::filesystem::current_path()),
    });
}

VfsResult<void> Vfs::Mount(std::string_view mountPath, VfsProviderDesc provider)
{
    const std::string normalized = NormalizePath(mountPath);

    if (normalized == "/")
        return std::unexpected(VfsError{
            EVfsErrorCode::MountConflict,
            "Cannot mount over the root provider"
            });

    if (!normalized.starts_with('/'))
        return std::unexpected(VfsError{
            EVfsErrorCode::MountConflict,
            std::format("Mount path must be absolute (start with '/'): {}", normalized)
            });

    std::scoped_lock<std::mutex> lock(m_mutex);

    for (const auto& mp : m_mounts)
    {
        if (mp.path == normalized)
            return std::unexpected(VfsError{
                EVfsErrorCode::MountConflict,
                std::format("A provider is already mounted at: {}", normalized)
                });
    }

    const std::string_view top = TopComponent(normalized);
    const MountPoint* root = m_Resolve("/");
    if (root && root->provider.exists(top))
        return std::unexpected(VfsError{
            EVfsErrorCode::MountConflict,
            std::format("A filesystem entry already exists at: /{}", std::string(top))
            });

    auto it = std::ranges::find_if(m_mounts, [&](const MountPoint& mp) {
        return mp.path.size() < normalized.size();
        });

    m_mounts.insert(it, MountPoint{
        .path = normalized,
        .provider = std::move(provider)
        });

    return {};
}

VfsResult<void> Vfs::Unmount(std::string_view mountPath)
{
    const std::string normalized = NormalizePath(mountPath);

    if (normalized == "/")
        return std::unexpected(VfsError{
            EVfsErrorCode::MountConflict,
            "Cannot unmount the root provider"
            });

    std::scoped_lock<std::mutex> lock(m_mutex);

    auto it = std::ranges::find_if(m_mounts, [&](const MountPoint& mp) {
        return mp.path == normalized;
        });

    if (it == m_mounts.end())
        return std::unexpected(VfsError{
            EVfsErrorCode::NotFound,
            std::format("No provider mounted at: {}", normalized)
            });

    m_mounts.erase(it);
    return {};
}

VfsResult<VfsFile> Vfs::Open(std::string_view path) const
{
    const std::string normalized = NormalizePath(path);

    std::scoped_lock<std::mutex> lock(m_mutex);

    const MountPoint* mp = m_Resolve(normalized);
    if (!mp)
        return std::unexpected(VfsError{
            EVfsErrorCode::NotFound,
            std::format("No provider for: {}", normalized)
            });

    const std::string relative = m_StripPrefix(mp->path, normalized);

    return mp->provider.open(relative).transform([&path](VfsFileDesc&& desc) {
        std::println("VFS: Opening: {}", path);
        return VfsFile(std::move(desc));
    });
}

VfsResult<std::vector<VfsEntry>> Vfs::List(std::string_view directory) const
{
    const std::string normalized = NormalizePath(directory);

    std::scoped_lock<std::mutex> lock(m_mutex);

    // Find the primary provider for this path.
    const MountPoint* primary = m_Resolve(normalized);
    if (!primary)
        return std::unexpected(VfsError{
            EVfsErrorCode::NotFound,
            std::format("No provider for: {}", normalized)
            });

    const std::string relative = m_StripPrefix(primary->path, normalized);
    auto result = primary->provider.list(relative);
    if (!result)
        return result;

    std::vector<VfsEntry>& entries = *result;

    for (const auto& mp : m_mounts)
    {
        if (mp.path == "/" || mp.path == normalized)
            continue;

        const std::string_view prefix = (normalized == "/")
            ? std::string_view("/")
            : std::string_view(normalized);

        if (!mp.path.starts_with(prefix))
            continue;

        const std::size_t restStart = (normalized == "/") ? 1 : normalized.size() + 1;
        if (restStart > mp.path.size())
            continue;

        const std::string_view rest = std::string_view(mp.path).substr(restStart);
        if (rest.empty() || rest.find('/') != std::string_view::npos)
            continue;

        std::erase_if(entries, [&](const VfsEntry& e) { return e.name == rest; });

        entries.push_back(VfsEntry{
            .name = std::string(rest),
            .kind = EVfsEntryKind::Directory,
            .size = 0,
            });
    }

    return result;
}

bool Vfs::Exists(std::string_view path) const
{
    const std::string normalized = NormalizePath(path);

    std::scoped_lock<std::mutex> lock(m_mutex);

    const MountPoint* mp = m_Resolve(normalized);
    if (!mp)
        return false;

    // A path that exactly matches a mount point always exists as a directory
    if (mp->path == normalized && normalized != "/")
        return true;

    const std::string relative = m_StripPrefix(mp->path, normalized);
    return mp->provider.exists(relative);
}

