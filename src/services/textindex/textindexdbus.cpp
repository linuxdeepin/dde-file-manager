// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "private/textindexdbus_p.h"
#include "utils/indexutility.h"
#include "utils/systemdcpuutils.h"
#include "utils/textindexconfig.h"

#include <QDir>

static constexpr char kTextIndexObjPath[] { "/org/deepin/Filemanager/TextIndex" };

SERVICETEXTINDEX_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(SERVICETEXTINDEX_NAMESPACE)
SERVICETEXTINDEX_END_NAMESPACE

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

void TextIndexDBusPrivate::initialize()
{
    fsEventController->setupFSEventCollector();
    initializeSupportedExtensions();
}

void TextIndexDBusPrivate::initConnect()
{
    QObject::connect(taskManager, &TaskManager::taskFinished,
                     q, [this](const QString &type, const QString &path, bool success) {
                         QString msg;
                         fmInfo() << "TextIndexDBus: Resetting CPU limit after task completion";
                         if (!SystemdCpuUtils::resetCpuQuota(Defines::kTextIndexServiceName, &msg)) {
                             fmWarning() << "TextIndexDBus: Failed to reset CPU quota:" << msg;
                         }
                         emit q->TaskFinished(type, path, success);
                     });

    QObject::connect(taskManager, &TaskManager::taskProgressChanged,
                     q, [this](const QString &type, const QString &path, qint64 count, qint64 total) {
                         emit q->TaskProgressChanged(type, path, count, total);
                     });

    QObject::connect(fsEventController, &FSEventController::requestProcessFileChanges,
                     q, &TextIndexDBus::ProcessFileChanges);
    QObject::connect(fsEventController, &FSEventController::requestProcessFileMoves,
                     q, &TextIndexDBus::ProcessFileMoves);
    QObject::connect(fsEventController, &FSEventController::monitoring,
                     q, [this](bool start) {
                         handleMonitoring(start);
                     });
    QObject::connect(fsEventController, &FSEventController::requestSlientStart,
                     q, [this]() {
                         handleSlientStart();
                     });

    // Connect to TextIndexConfig changes
    QObject::connect(&TextIndexConfig::instance(), &TextIndexConfig::configChanged,
                     q, [this]() {
                         handleConfigChanged();
                     });
}

void TextIndexDBusPrivate::handleMonitoring(bool start)
{
    fmInfo() << "TextIndexDBus: FS event monitoring state changed to:" << start;
    if (!start) {
        fsEventController->stopFSMonitoring();
        return;
    }

    fsEventController->startFSMonitoring();
}

void TextIndexDBusPrivate::handleSlientStart()
{
    // NOTE: Used only for silent updates after the service is started for the first time!
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        // Create or update indexes silently
        const auto &configuredDirs = DFMSEARCH::Global::defaultIndexedDirectory();
        QStringList pathsToProcess;

        if (configuredDirs.isEmpty()) {
            pathsToProcess.append(QDir::homePath());
        } else {
            pathsToProcess = configuredDirs;   // Assuming configuredDirs is a QStringList or compatible
        }

        if (!canSilentlyRefreshIndex(pathsToProcess.first())) {
            fmWarning() << "TextIndexDBus: Unable to refresh index, task already running for:" << pathsToProcess.first();
            return;
        }

        fmInfo() << "TextIndexDBus: Starting silent index task for:" << pathsToProcess;

        if (q->IndexDatabaseExists()) {   // update
            taskManager->startTask(IndexTask::Type::Update, pathsToProcess, true);
        } else {   // create
            taskManager->startTask(IndexTask::Type::Create, pathsToProcess, true);
        }
    });
}

bool TextIndexDBusPrivate::canSilentlyRefreshIndex(const QString &path) const
{
    if (auto taskTypeOpt = taskManager->currentTaskType(); taskTypeOpt.has_value()) {
        if (auto taskPathOpt = taskManager->currentTaskPath(); taskPathOpt.has_value()) {
            const auto &type = *taskTypeOpt;
            const auto &taskPath = *taskPathOpt;

            if ((type == IndexTask::Type::Create || type == IndexTask::Type::Update) && (taskPath == path)) {
                return false;
            }
        }
    }

    return true;
}

TextIndexDBus::TextIndexDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), d(new TextIndexDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString(name)).registerObject(kTextIndexObjPath, this, opts);
}

TextIndexDBus::~TextIndexDBus() { }

void TextIndexDBus::cleanup()
{
    d->fsEventController->setEnabledNow(false);
    StopCurrentTask();
}

void TextIndexDBus::Init()
{
    // 预防启动时没有开启全文检索，后续手动去开启全文检索，将造成 2 次索引
    d->fsEventController->setSilentlyRefreshStarted(true);
}

bool TextIndexDBus::IsEnabled()
{
    return d->fsEventController->isEnabled();
}

void TextIndexDBus::SetEnabled(bool enabled)
{
    d->fsEventController->setEnabled(enabled);
}

bool TextIndexDBus::CreateIndexTask(const QStringList &paths)
{
    return d->taskManager->startTask(IndexTask::Type::Create, paths);
}

bool TextIndexDBus::UpdateIndexTask(const QStringList &paths)
{
    return d->taskManager->startTask(IndexTask::Type::Update, paths);
}

bool TextIndexDBus::StopCurrentTask()
{
    if (!d->taskManager->hasRunningTask())
        return false;

    d->taskManager->stopCurrentTask();
    return true;
}

bool TextIndexDBus::HasRunningTask()
{
    return d->taskManager->hasRunningTask();
}

bool TextIndexDBus::IndexDatabaseExists()
{
    // First check if the index files exist
    if (!DFMSEARCH::Global::isContentIndexAvailable()) {
        return false;
    }

    // Then check if the version is compatible
    if (!IndexUtility::isCompatibleVersion()) {
        fmWarning() << "TextIndexDBus: Index database exists but version is incompatible."
                    << "Current version:" << Defines::kIndexVersion
                    << "Stored version:" << IndexUtility::getIndexVersion()
                    << "Index considered invalid due to version mismatch";
        return false;
    }

    if (IndexUtility::getLastUpdateTime().isEmpty()) {
        fmWarning() << "TextIndexDBus: Last update time is empty, index may be corrupted";
        return false;
    }

    return true;
}

QString TextIndexDBus::GetLastUpdateTime()
{
    return IndexUtility::getLastUpdateTime();
}

bool TextIndexDBus::ProcessFileChanges(const QStringList &createdFiles,
                                       const QStringList &modifiedFiles,
                                       const QStringList &deletedFiles)
{
    bool tasksQueued = false;

    // 处理删除的文件（优先处理，因为这些文件可能已经不存在）
    if (!deletedFiles.isEmpty()) {
        fmInfo() << "TextIndexDBus: Processing" << deletedFiles.size() << "deleted files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::RemoveFileList, deletedFiles, true) || tasksQueued;
    }

    // 处理新增的文件
    if (!createdFiles.isEmpty()) {
        fmInfo() << "TextIndexDBus: Processing" << createdFiles.size() << "created files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::CreateFileList, createdFiles, true) || tasksQueued;
    }

    // 处理修改的文件
    if (!modifiedFiles.isEmpty()) {
        fmInfo() << "TextIndexDBus: Processing" << modifiedFiles.size() << "modified files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::UpdateFileList, modifiedFiles, true) || tasksQueued;
    }

    return tasksQueued;
}

bool TextIndexDBus::ProcessFileMoves(const QHash<QString, QString> &movedFiles)
{
    if (movedFiles.isEmpty()) {
        fmDebug() << "TextIndexDBus: No file moves to process";
        return false;
    }

    fmInfo() << "TextIndexDBus: Processing" << movedFiles.size() << "moved files";
    
    // 启动文件移动任务
    bool taskQueued = d->taskManager->startFileMoveTask(movedFiles, true);
    
    return taskQueued;
}

void TextIndexDBusPrivate::initializeSupportedExtensions()
{
    m_currentSupportedExtensions = TextIndexConfig::instance().supportedFileExtensions();
    fmInfo() << "TextIndexDBus: Initialized supported file extensions (" << m_currentSupportedExtensions.size() << "):"
             << m_currentSupportedExtensions;
}

void TextIndexDBusPrivate::handleConfigChanged()
{
    const auto newSupportedExtensions = TextIndexConfig::instance().supportedFileExtensions();

    // Convert to sets for order-insensitive comparison
    const QSet<QString> currentExtensionsSet(m_currentSupportedExtensions.begin(), m_currentSupportedExtensions.end());
    const QSet<QString> newExtensionsSet(newSupportedExtensions.begin(), newSupportedExtensions.end());

    // Check if supported file extensions have changed (order-insensitive)
    if (currentExtensionsSet != newExtensionsSet) {
        fmInfo() << "TextIndexDBus: Supported file extensions changed from" << m_currentSupportedExtensions.size()
                 << "to" << newSupportedExtensions.size() << "extensions";

        // Update stored extensions
        m_currentSupportedExtensions = newSupportedExtensions;

        // Trigger index update for configured directories
        const auto &configuredDirs = DFMSEARCH::Global::defaultIndexedDirectory();
        QStringList pathsToProcess;

        if (configuredDirs.isEmpty()) {
            pathsToProcess.append(QDir::homePath());
        } else {
            pathsToProcess = configuredDirs;
        }

        // Only start update task if index database exists
        if (q->IndexDatabaseExists()) {
            fmInfo() << "TextIndexDBus: Starting index update task due to supported file extensions change for paths:" << pathsToProcess;
            taskManager->startTask(IndexTask::Type::Update, pathsToProcess);
        } else {
            fmWarning() << "TextIndexDBus: Cannot start index update task, index database does not exist";
        }
    }
}
