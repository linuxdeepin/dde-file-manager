// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrindexdbus.h"
#include "private/ocrindexdbus_p.h"
#include "utils/indexutility.h"
#include "utils/systemdcpuutils.h"
#include "utils/textindexconfig.h"

#include <QDir>
#include <QDBusConnection>

SERVICETEXTINDEX_USE_NAMESPACE

namespace {

QStringList defaultPathsToProcess()
{
    const auto &configuredDirs = DFMSEARCH::Global::defaultIndexedDirectory();
    if (configuredDirs.isEmpty()) {
        return { QDir::homePath() };
    }

    return configuredDirs;
}

}   // namespace

void OcrIndexDBusPrivate::initialize()
{
    runtime->fsEventController()->setupFSEventCollector();
    initializeSupportedExtensions();

    // Check for dirty state at startup and set recovery pending flag
    // This must be done before any incremental task can complete and clear the Dirty state
    const IndexUtility::IndexState state = runtime->stateStore().getIndexState();
    if (state != IndexUtility::IndexState::Clean) {
        fmInfo() << "OcrIndexDBus: Dirty state detected at startup, setting recovery pending flag";
        runtime->taskManager()->setRecoveryPending(true);
    }
}

void OcrIndexDBusPrivate::initConnect()
{
    QObject::connect(runtime->taskManager(), &TaskManager::taskFinished,
                     q, [this](const QString &type, const QString &path, bool success) {
                         QString msg;
                         fmDebug() << "OcrIndexDBus: Resetting CPU limit after task completion";
                         if (!SystemdCpuUtils::resetCpuQuota(Defines::kTextIndexServiceName, &msg)) {
                             fmWarning() << "OcrIndexDBus: Failed to reset CPU quota:" << msg;
                         }
                         emit q->TaskFinished(type, path, success);
                     });

    QObject::connect(runtime->taskManager(), &TaskManager::taskProgressChanged,
                     q, [this](const QString &type, const QString &path, qint64 count, qint64 total) {
                         emit q->TaskProgressChanged(type, path, count, total);
                     });

    QObject::connect(runtime->fsEventController(), &FSEventController::requestProcessFileChanges,
                     q, &OcrIndexDBus::ProcessFileChanges);
    QObject::connect(runtime->fsEventController(), &FSEventController::requestProcessFileMoves,
                     q, &OcrIndexDBus::ProcessFileMoves);
    QObject::connect(runtime->fsEventController(), &FSEventController::monitoring,
                     q, [this](bool start) {
                         handleMonitoring(start);
                     });
    QObject::connect(runtime->fsEventController(), &FSEventController::requestSlientStart,
                     q, [this]() {
                         handleSlientStart();
                     });

    QObject::connect(&TextIndexConfig::instance(), &TextIndexConfig::configChanged,
                     q, [this]() {
                         handleConfigChanged();
                     });
    QObject::connect(IndexUtility::AnythingConfigWatcher::instance(), &IndexUtility::AnythingConfigWatcher::rebuildRequired,
                     q, [this](const QString &reason) {
                         fmInfo() << "OcrIndexDBus: ANYTHING config changed, marking rebuild required. reason:" << reason;
                         runtime->stateStore().setNeedsRebuild(true);
                     });
    QObject::connect(IndexUtility::DlnfsConfigWatcher::instance(), &IndexUtility::DlnfsConfigWatcher::rebuildRequired,
                     q, [this](const QString &reason) {
                         fmInfo() << "OcrIndexDBus: DLNFS config changed, marking rebuild required. reason:" << reason;
                         runtime->stateStore().setNeedsRebuild(true);
                     });
}

void OcrIndexDBusPrivate::handleMonitoring(bool start)
{
    fmInfo() << "OcrIndexDBus: FS event monitoring state changed to:" << start;
    if (!start) {
        runtime->fsEventController()->stopFSMonitoring();
        return;
    }

    runtime->fsEventController()->startFSMonitoring();
}

void OcrIndexDBusPrivate::handleSlientStart()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        const QStringList pathsToProcess = defaultPathsToProcess();
        if (pathsToProcess.isEmpty()) {
            fmWarning() << "OcrIndexDBus: No configured paths available for silent refresh";
            return;
        }

        if (!canSilentlyRefreshIndex(pathsToProcess.first())) {
            fmWarning() << "OcrIndexDBus: Unable to refresh index, task already running for:" << pathsToProcess.first();
            return;
        }

        if (!q->IndexDatabaseExists()) {
            fmInfo() << "OcrIndexDBus: Index database does not exist, starting create task for:" << pathsToProcess;
            runtime->taskManager()->startTask(IndexTask::Type::Create, pathsToProcess, true);
            return;
        }

        // Use recoveryPending flag which was set at startup if Dirty state was detected
        const bool needsRecovery = runtime->taskManager()->isRecoveryPending();
        const bool needsRebuild = runtime->stateStore().needsRebuild();

        if (needsRebuild) {
            fmInfo() << "OcrIndexDBus: Config changed, clearing needsRebuild flag";
            runtime->stateStore().setNeedsRebuild(false);
        }

        if (needsRebuild || needsRecovery) {
            fmInfo() << "OcrIndexDBus: Starting update task - needsRebuild:" << needsRebuild
                     << "needsRecovery:" << needsRecovery << "for:" << pathsToProcess;
            runtime->taskManager()->startTask(IndexTask::Type::Update, pathsToProcess, true);
            return;
        }

        fmInfo() << "OcrIndexDBus: Clean state and no config changes, skipping global update";
    });
}

bool OcrIndexDBusPrivate::canSilentlyRefreshIndex(const QString &path) const
{
    if (auto taskTypeOpt = runtime->taskManager()->currentTaskType(); taskTypeOpt.has_value()) {
        if (auto taskPathOpt = runtime->taskManager()->currentTaskPath(); taskPathOpt.has_value()) {
            const auto &type = *taskTypeOpt;
            const auto &taskPath = *taskPathOpt;

            if ((type == IndexTask::Type::Create || type == IndexTask::Type::Update) && (taskPath == path)) {
                return false;
            }
        }
    }

    return true;
}

OcrIndexDBus::OcrIndexDBus(QObject *parent)
    : QObject(parent), QDBusContext(), d(new OcrIndexDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection::sessionBus().registerObject(Defines::kOcrIndexDBusObjectPath, this, opts);
}

OcrIndexDBus::~OcrIndexDBus() { }

void OcrIndexDBus::cleanup()
{
    d->runtime->fsEventController()->setEnabledNow(false);

    const bool hasUnfinishedWork = d->runtime->taskManager()->hasRunningTask()
            || d->runtime->taskManager()->hasQueuedTasks();
    if (hasUnfinishedWork) {
        fmWarning() << "OcrIndexDBus: Service cleanup with unfinished indexing work, marking state as dirty";
        d->runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    }

    StopCurrentTask();
}

void OcrIndexDBus::Init()
{
    d->runtime->fsEventController()->setSilentlyRefreshStarted(true);
}

bool OcrIndexDBus::IsEnabled()
{
    return d->runtime->fsEventController()->isEnabled();
}

void OcrIndexDBus::SetEnabled(bool enabled)
{
    d->runtime->fsEventController()->setEnabled(enabled);
}

bool OcrIndexDBus::CreateIndexTask(const QStringList &paths)
{
    return d->runtime->taskManager()->startTask(IndexTask::Type::Create, paths);
}

bool OcrIndexDBus::UpdateIndexTask(const QStringList &paths)
{
    return d->runtime->taskManager()->startTask(IndexTask::Type::Update, paths);
}

bool OcrIndexDBus::StopCurrentTask()
{
    if (!d->runtime->taskManager()->hasRunningTask()) {
        return false;
    }

    d->runtime->taskManager()->stopCurrentTask();
    return true;
}

bool OcrIndexDBus::HasRunningTask()
{
    return d->runtime->taskManager()->hasRunningTask();
}

bool OcrIndexDBus::IndexDatabaseExists()
{
    if (!d->runtime->profile().isIndexAvailable()) {
        return false;
    }

    if (!d->runtime->stateStore().isCompatibleVersion()) {
        fmWarning() << "OcrIndexDBus: Index database exists but version is incompatible."
                    << "Current version:" << d->runtime->profile().runtimeIndexVersion()
                    << "Stored version:" << d->runtime->stateStore().getIndexVersion();
        return false;
    }

    if (d->runtime->stateStore().getLastUpdateTime().isEmpty()) {
        fmWarning() << "OcrIndexDBus: Last update time is empty, index may be corrupted";
        return false;
    }

    return true;
}

QString OcrIndexDBus::GetLastUpdateTime()
{
    return d->runtime->stateStore().getLastUpdateTime();
}

bool OcrIndexDBus::ProcessFileChanges(const QStringList &createdFiles,
                                      const QStringList &modifiedFiles,
                                      const QStringList &deletedFiles)
{
    bool tasksQueued = false;

    if (!deletedFiles.isEmpty()) {
        fmInfo() << "OcrIndexDBus: Processing" << deletedFiles.size() << "deleted files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::RemoveFileList, deletedFiles, true) || tasksQueued;
    }

    if (!createdFiles.isEmpty()) {
        fmInfo() << "OcrIndexDBus: Processing" << createdFiles.size() << "created files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::CreateFileList, createdFiles, true) || tasksQueued;
    }

    if (!modifiedFiles.isEmpty()) {
        fmInfo() << "OcrIndexDBus: Processing" << modifiedFiles.size() << "modified files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::UpdateFileList, modifiedFiles, true) || tasksQueued;
    }

    return tasksQueued;
}

bool OcrIndexDBus::ProcessFileMoves(const QHash<QString, QString> &movedFiles)
{
    if (movedFiles.isEmpty()) {
        fmDebug() << "OcrIndexDBus: No file moves to process";
        return false;
    }

    fmInfo() << "OcrIndexDBus: Processing" << movedFiles.size() << "moved files";
    return d->runtime->taskManager()->startFileMoveTask(movedFiles, true);
}

void OcrIndexDBusPrivate::initializeSupportedExtensions()
{
    m_currentSupportedExtensions = TextIndexConfig::instance().supportedOcrImageExtensions();
    fmInfo() << "OcrIndexDBus: Initialized supported OCR image extensions (" << m_currentSupportedExtensions.size() << "):"
             << m_currentSupportedExtensions;
}

void OcrIndexDBusPrivate::handleConfigChanged()
{
    const auto newSupportedExtensions = TextIndexConfig::instance().supportedOcrImageExtensions();
    const QSet<QString> currentExtensionsSet(m_currentSupportedExtensions.begin(), m_currentSupportedExtensions.end());
    const QSet<QString> newExtensionsSet(newSupportedExtensions.begin(), newSupportedExtensions.end());

    if (currentExtensionsSet != newExtensionsSet) {
        fmInfo() << "OcrIndexDBus: Supported OCR image extensions changed from" << m_currentSupportedExtensions.size()
                 << "to" << newSupportedExtensions.size() << "extensions";
        m_currentSupportedExtensions = newSupportedExtensions;

        const QStringList pathsToProcess = defaultPathsToProcess();
        if (q->IndexDatabaseExists()) {
            fmInfo() << "OcrIndexDBus: Starting index update task due to supported OCR image extensions change for paths:" << pathsToProcess;
            runtime->taskManager()->startTask(IndexTask::Type::Update, pathsToProcess);
        } else {
            fmWarning() << "OcrIndexDBus: Cannot start index update task, index database does not exist";
        }
    }
}
