// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2026 PieJam Fork Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <piejam/gui/model/FileBrowser.h>

#include <piejam/file_manager/file_browser_controller.h>
#include <piejam/file_manager/recording_list_model.h>

#include <QAbstractListModel>

namespace piejam::gui::model
{

struct FileBrowser::impl
{
    std::unique_ptr<file_manager::FileBrowserController> controller;
};

FileBrowser::FileBrowser(
    std::string const& recordings_path,
    std::string const& db_path,
    QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<impl>())
{
    m_impl->controller = std::make_unique<file_manager::FileBrowserController>(
        recordings_path,
        db_path,
        this);

    connectSignals();
    updateProperties();
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::connectSignals()
{
    auto* ctrl = m_impl->controller.get();

    // Forward all controller signals
    connect(ctrl, &file_manager::FileBrowserController::storageInfoChanged,
            this, &FileBrowser::updateProperties);

    connect(ctrl, &file_manager::FileBrowserController::isScanningChanged,
            this, [this, ctrl]() {
                setIsScanning(ctrl->isScanning());
            });

    connect(ctrl, &file_manager::FileBrowserController::recordingCountChanged,
            this, [this, ctrl]() {
                setRecordingCount(ctrl->recordingCount());
            });

    connect(ctrl, &file_manager::FileBrowserController::tagsChanged,
            this, [this, ctrl]() {
                setAllTags(ctrl->allTags());
            });

    connect(ctrl, &file_manager::FileBrowserController::scanComplete,
            this, &FileBrowser::scanComplete);

    connect(ctrl, &file_manager::FileBrowserController::scanProgress,
            this, &FileBrowser::scanProgress);

    connect(ctrl, &file_manager::FileBrowserController::integrityCheckComplete,
            this, &FileBrowser::integrityCheckComplete);

    connect(ctrl, &file_manager::FileBrowserController::recordingDeleted,
            this, &FileBrowser::recordingDeleted);

    connect(ctrl, &file_manager::FileBrowserController::operationError,
            this, &FileBrowser::operationError);

    connect(ctrl, &file_manager::FileBrowserController::newRecordingDetected,
            this, &FileBrowser::newRecordingDetected);
}

void FileBrowser::updateProperties()
{
    auto* ctrl = m_impl->controller.get();

    setStorageInfo(ctrl->storageInfo());
    setUsedBytes(ctrl->usedBytes());
    setTotalBytes(ctrl->totalBytes());
    setUsedPercent(ctrl->usedPercent());
    setIsScanning(ctrl->isScanning());
    setRecordingCount(ctrl->recordingCount());
    setAllTags(ctrl->allTags());
    setRecordingsPath(ctrl->recordingsPath());
}

QAbstractListModel* FileBrowser::recordings() const
{
    return m_impl->controller->recordings();
}

QVariantMap FileBrowser::getRecordingDetails(int index) const
{
    return m_impl->controller->getRecordingDetails(index);
}

void FileBrowser::refresh()
{
    m_impl->controller->refresh();
    updateProperties();
}

void FileBrowser::rescan()
{
    m_impl->controller->rescan();
}

void FileBrowser::deleteRecording(int index)
{
    m_impl->controller->deleteRecording(index);
}

void FileBrowser::updateTags(int index, QStringList const& tags)
{
    m_impl->controller->updateTags(index, tags);
}

void FileBrowser::updateNotes(int index, QString const& notes)
{
    m_impl->controller->updateNotes(index, notes);
}

void FileBrowser::updateRating(int index, int rating)
{
    m_impl->controller->updateRating(index, rating);
}

void FileBrowser::updateExportStatus(int index, int status)
{
    m_impl->controller->updateExportStatus(index, status);
}

void FileBrowser::verifyIntegrity(int index)
{
    m_impl->controller->verifyIntegrity(index);
}

void FileBrowser::addScanDirectory(QString const& path)
{
    m_impl->controller->addScanDirectory(path);
}

void FileBrowser::removeScanDirectory(QString const& path)
{
    m_impl->controller->removeScanDirectory(path);
}

QStringList FileBrowser::scanDirectories() const
{
    return m_impl->controller->scanDirectories();
}

void FileBrowser::filterByTag(QString const& tag)
{
    m_impl->controller->filterByTag(tag);
}

void FileBrowser::filterByMinRating(int minRating)
{
    m_impl->controller->filterByMinRating(minRating);
}

void FileBrowser::filterByStatus(int status)
{
    m_impl->controller->filterByStatus(status);
}

void FileBrowser::clearFilters()
{
    m_impl->controller->clearFilters();
}

} // namespace piejam::gui::model
