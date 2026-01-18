// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/file_manager/recording_scanner.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtConcurrent>

#include <algorithm>
#include <mutex>
#include <set>

namespace piejam::file_manager
{

struct recording_scanner::impl
{
    std::vector<std::string> scan_directories;
    std::unique_ptr<QFileSystemWatcher> watcher;
    bool auto_scan_enabled{false};

    new_file_callback on_new_file;
    file_removed_callback on_file_removed;

    std::set<std::string> known_files;
    std::mutex mutex;
};

recording_scanner::recording_scanner(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<impl>())
{
    m_impl->watcher = std::make_unique<QFileSystemWatcher>(this);

    connect(m_impl->watcher.get(), &QFileSystemWatcher::directoryChanged,
            this, &recording_scanner::onDirectoryChanged);
    connect(m_impl->watcher.get(), &QFileSystemWatcher::fileChanged,
            this, &recording_scanner::onFileChanged);
}

recording_scanner::~recording_scanner() = default;

recording_scanner::recording_scanner(recording_scanner&&) noexcept = default;
recording_scanner& recording_scanner::operator=(recording_scanner&&) noexcept = default;

void
recording_scanner::add_scan_directory(std::string const& path)
{
    std::lock_guard lock(m_impl->mutex);

    auto it = std::find(m_impl->scan_directories.begin(),
                        m_impl->scan_directories.end(), path);
    if (it == m_impl->scan_directories.end())
    {
        m_impl->scan_directories.push_back(path);

        if (m_impl->auto_scan_enabled)
        {
            m_impl->watcher->addPath(QString::fromStdString(path));
        }
    }
}

void
recording_scanner::remove_scan_directory(std::string const& path)
{
    std::lock_guard lock(m_impl->mutex);

    auto it = std::find(m_impl->scan_directories.begin(),
                        m_impl->scan_directories.end(), path);
    if (it != m_impl->scan_directories.end())
    {
        m_impl->scan_directories.erase(it);
        m_impl->watcher->removePath(QString::fromStdString(path));
    }
}

auto
recording_scanner::scan_directories() const -> std::vector<std::string>
{
    std::lock_guard lock(m_impl->mutex);
    return m_impl->scan_directories;
}

auto
recording_scanner::scan() -> std::vector<std::string>
{
    emit scanStarted();

    std::vector<std::string> results;
    std::vector<std::string> dirs;

    {
        std::lock_guard lock(m_impl->mutex);
        dirs = m_impl->scan_directories;
    }

    int total = 0;
    int current = 0;

    // First pass: count files
    for (auto const& dir : dirs)
    {
        QDirIterator it(QString::fromStdString(dir),
                        QDir::Files | QDir::Readable,
                        QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            it.next();
            if (is_audio_file(it.filePath().toStdString()))
            {
                ++total;
            }
        }
    }

    // Second pass: collect files with progress
    for (auto const& dir : dirs)
    {
        QDir qdir(QString::fromStdString(dir));
        if (!qdir.exists())
        {
            continue;
        }

        QDirIterator it(QString::fromStdString(dir),
                        QDir::Files | QDir::Readable,
                        QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            it.next();
            std::string file_path = it.filePath().toStdString();

            if (is_audio_file(file_path))
            {
                results.push_back(file_path);
                ++current;
                emit scanProgress(current, total);
            }
        }
    }

    // Update known files
    {
        std::lock_guard lock(m_impl->mutex);
        m_impl->known_files.clear();
        for (auto const& f : results)
        {
            m_impl->known_files.insert(f);
        }
    }

    emit scanComplete(static_cast<int>(results.size()));
    return results;
}

void
recording_scanner::scan_async(scan_complete_callback callback)
{
    QtConcurrent::run([this, callback]() {
        auto results = scan();
        if (callback)
        {
            callback(results);
        }
    });
}

void
recording_scanner::set_auto_scan(bool enable)
{
    std::lock_guard lock(m_impl->mutex);

    if (m_impl->auto_scan_enabled == enable)
    {
        return;
    }

    m_impl->auto_scan_enabled = enable;

    if (enable)
    {
        for (auto const& dir : m_impl->scan_directories)
        {
            m_impl->watcher->addPath(QString::fromStdString(dir));
        }
    }
    else
    {
        QStringList paths = m_impl->watcher->directories();
        if (!paths.isEmpty())
        {
            m_impl->watcher->removePaths(paths);
        }
    }
}

auto
recording_scanner::auto_scan_enabled() const -> bool
{
    std::lock_guard lock(m_impl->mutex);
    return m_impl->auto_scan_enabled;
}

auto
recording_scanner::is_audio_file(std::string const& path) -> bool
{
    static std::set<std::string> const extensions = {
        ".wav", ".WAV",
        ".flac", ".FLAC",
        ".aiff", ".AIFF", ".aif", ".AIF",
        ".ogg", ".OGG"
    };

    auto dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos)
    {
        return false;
    }

    std::string ext = path.substr(dot_pos);
    return extensions.find(ext) != extensions.end();
}

void
recording_scanner::set_new_file_callback(new_file_callback callback)
{
    m_impl->on_new_file = std::move(callback);
}

void
recording_scanner::set_file_removed_callback(file_removed_callback callback)
{
    m_impl->on_file_removed = std::move(callback);
}

auto
recording_scanner::supported_extensions() -> std::vector<std::string>
{
    return {".wav", ".flac", ".aiff", ".aif", ".ogg"};
}

void
recording_scanner::onDirectoryChanged(QString const& path)
{
    // Rescan the directory to find new/removed files
    QDir dir(path);
    if (!dir.exists())
    {
        return;
    }

    std::set<std::string> current_files;

    QDirIterator it(path, QDir::Files | QDir::Readable);
    while (it.hasNext())
    {
        it.next();
        std::string file_path = it.filePath().toStdString();
        if (is_audio_file(file_path))
        {
            current_files.insert(file_path);
        }
    }

    std::lock_guard lock(m_impl->mutex);

    // Find new files
    for (auto const& f : current_files)
    {
        if (m_impl->known_files.find(f) == m_impl->known_files.end())
        {
            m_impl->known_files.insert(f);
            emit newFileDetected(QString::fromStdString(f));

            if (m_impl->on_new_file)
            {
                m_impl->on_new_file(f);
            }
        }
    }

    // Find removed files (only those that were in this directory)
    std::vector<std::string> removed;
    for (auto const& f : m_impl->known_files)
    {
        if (QString::fromStdString(f).startsWith(path))
        {
            if (current_files.find(f) == current_files.end())
            {
                removed.push_back(f);
            }
        }
    }

    for (auto const& f : removed)
    {
        m_impl->known_files.erase(f);
        emit fileRemoved(QString::fromStdString(f));

        if (m_impl->on_file_removed)
        {
            m_impl->on_file_removed(f);
        }
    }
}

void
recording_scanner::onFileChanged(QString const& path)
{
    std::string file_path = path.toStdString();

    // Check if file still exists
    QFileInfo info(path);
    if (!info.exists())
    {
        std::lock_guard lock(m_impl->mutex);
        m_impl->known_files.erase(file_path);
        emit fileRemoved(path);

        if (m_impl->on_file_removed)
        {
            m_impl->on_file_removed(file_path);
        }
    }
}

} // namespace piejam::file_manager
