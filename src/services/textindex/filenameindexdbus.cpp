// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameindexdbus.h"
#include "private/filenameindexdbus_p.h"
#include "utils/filetypemapper.h"
#include "utils/indexutility.h"
#include "utils/pinyinprocessor.h"

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

void FileNameIndexDBusPrivate::initialize()
{
    runtime->fsEventController()->setupFSEventCollector();

    // 在主线程预热依赖 DConfig / 字典文件的单例：避免首次在 worker 线程构造时
    // 跨线程创建 DConfig（DConfig 内部走 DBus，跨线程亲和性风险）或读字典。
    PinyinProcessor::instance();
    FileTypeMapper::instance();

    // Mark silently-refresh-started so that the first SetEnabled(true) triggers
    // the silent index update path. Previously this was done via a separate
    // DBus "Init" method called by the daemon; now it is set at construction
    // time because the service process is always started fresh by the daemon.
    runtime->fsEventController()->setSilentlyRefreshStarted(true);

    // Check for dirty state at startup and set recovery pending flag
    // This must be done before any incremental task can complete and clear the Dirty state
    const IndexUtility::IndexState state = runtime->stateStore().getIndexState();
    if (state != IndexUtility::IndexState::Clean) {
        fmInfo() << "FileNameIndexDBus: Dirty state detected at startup, setting recovery pending flag";
        runtime->taskManager()->setRecoveryPending(true);
    }
}

void FileNameIndexDBusPrivate::initConnect()
{
    QObject::connect(runtime->taskManager(), &TaskManager::taskFinished,
                     q, [this](const QString &type, const QString &path, bool success) {
                         emit q->TaskFinished(type, path, success);
                     });

    QObject::connect(runtime->taskManager(), &TaskManager::taskProgressChanged,
                     q, [this](const QString &type, const QString &path, qint64 count, qint64 total) {
                         emit q->TaskProgressChanged(type, path, count, total);
                     });

    QObject::connect(runtime->taskManager(), &TaskManager::indexStatusChanged,
                     q, [this](const QString &state, const QString &grade) {
                         emit q->IndexStatusChanged(state, grade);
                     });

    QObject::connect(runtime->fsEventController(), &FSEventController::requestProcessFileChanges,
                     q, &FileNameIndexDBus::ProcessFileChanges);
    QObject::connect(runtime->fsEventController(), &FSEventController::requestProcessFileMoves,
                     q, &FileNameIndexDBus::ProcessFileMoves);
    QObject::connect(runtime->fsEventController(), &FSEventController::monitoring,
                     q, [this](bool start) {
                         handleMonitoring(start);
                     });
    QObject::connect(runtime->fsEventController(), &FSEventController::requestSilentStart,
                     q, [this]() {
                         handleSilentStart();
                     });

    // Lost-event recovery: schedule a compensating full update when the
    // monitor reports dropped filesystem events (dispatcher disconnect /
    // event queue overflow).
    QObject::connect(runtime->fsEventController(), &FSEventController::requestEventsRecovery,
                     q, [this]() {
                         const QStringList pathsToProcess = defaultPathsToProcess();
                         if (pathsToProcess.isEmpty()) {
                             fmWarning() << "FileNameIndexDBus: No configured paths available for events recovery";
                             return;
                         }

                         if (!q->IndexDatabaseExists()) {
                             fmInfo() << "FileNameIndexDBus: Index database does not exist, starting create task for events recovery:" << pathsToProcess;
                             runtime->taskManager()->startTask(IndexTask::Type::Create, pathsToProcess);
                             return;
                         }

                         fmInfo() << "FileNameIndexDBus: Starting update task for events recovery:" << pathsToProcess;
                         // 内部恢复：跳过全库清理（cleanupIndexs），见 UpdateIndexHandler 注释
                         runtime->taskManager()->startTask(IndexTask::Type::Update, pathsToProcess,
                                                           IndexTask::Grade::None, false, true);
                     });

    QObject::connect(IndexUtility::AnythingConfigWatcher::instance(), &IndexUtility::AnythingConfigWatcher::rebuildRequired,
                     q, [this](const QString &reason) {
                         fmInfo() << "FileNameIndexDBus: ANYTHING config changed, marking rebuild required. reason:" << reason;
                         runtime->stateStore().setNeedsRebuild(true);
                     });
    QObject::connect(IndexUtility::DlnfsConfigWatcher::instance(), &IndexUtility::DlnfsConfigWatcher::rebuildRequired,
                     q, [this](const QString &reason) {
                         fmInfo() << "FileNameIndexDBus: DLNFS config changed, marking rebuild required. reason:" << reason;
                         runtime->stateStore().setNeedsRebuild(true);
                     });
}

void FileNameIndexDBusPrivate::handleMonitoring(bool start)
{
    fmInfo() << "FileNameIndexDBus: FS event monitoring state changed to:" << start;
    if (!start) {
        runtime->fsEventController()->stopFSMonitoring();
        return;
    }

    runtime->fsEventController()->startFSMonitoring();
}

void FileNameIndexDBusPrivate::handleSilentStart()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        const QStringList pathsToProcess = defaultPathsToProcess();
        if (pathsToProcess.isEmpty()) {
            fmWarning() << "FileNameIndexDBus: No configured paths available for silent refresh";
            return;
        }

        if (!canSilentlyRefreshIndex(pathsToProcess.first())) {
            fmWarning() << "FileNameIndexDBus: Unable to refresh index, task already running for:" << pathsToProcess.first();
            return;
        }

        if (!q->IndexDatabaseExists()) {
            fmInfo() << "FileNameIndexDBus: Index database does not exist, starting create task for:" << pathsToProcess;
            runtime->taskManager()->startTask(IndexTask::Type::Create, pathsToProcess);
            return;
        }

        // Use recoveryPending flag which was set at startup if Dirty state was detected
        const bool needsRecovery = runtime->taskManager()->isRecoveryPending();
        const bool needsRebuild = runtime->stateStore().needsRebuild();

        if (needsRebuild) {
            fmInfo() << "FileNameIndexDBus: Config changed, clearing needsRebuild flag";
            runtime->stateStore().setNeedsRebuild(false);
        }

        if (needsRebuild || needsRecovery) {
            fmInfo() << "FileNameIndexDBus: Starting update task - needsRebuild:" << needsRebuild
                     << "needsRecovery:" << needsRecovery << "for:" << pathsToProcess;
            // 内部恢复（Dirty/needsRebuild）静默更新：跳过全库清理，见 UpdateIndexHandler 注释
            runtime->taskManager()->startTask(IndexTask::Type::Update, pathsToProcess,
                                              IndexTask::Grade::None, false, true);
            return;
        }

        fmInfo() << "FileNameIndexDBus: Clean state and no config changes, skipping global update";
    });
}

bool FileNameIndexDBusPrivate::canSilentlyRefreshIndex(const QString &path) const
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

FileNameIndexDBus::FileNameIndexDBus(QObject *parent)
    : QObject(parent), QDBusContext(), d(new FileNameIndexDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection::sessionBus().registerObject(Defines::kFileNameIndexDBusObjectPath, this, opts);
}

FileNameIndexDBus::~FileNameIndexDBus() { }

void FileNameIndexDBus::cleanup()
{
    d->runtime->fsEventController()->setEnabledNow(false);

    const bool hasUnfinishedWork = d->runtime->taskManager()->hasRunningTask()
            || d->runtime->taskManager()->hasQueuedTasks();
    if (hasUnfinishedWork) {
        fmWarning() << "FileNameIndexDBus: Service cleanup with unfinished indexing work, marking state as dirty";
        d->runtime->stateStore().setIndexState(IndexUtility::IndexState::Dirty);
    }

    StopCurrentTask();
}

bool FileNameIndexDBus::IsEnabled()
{
    return d->runtime->fsEventController()->isEnabled();
}

void FileNameIndexDBus::SetEnabled(bool enabled)
{
    d->runtime->fsEventController()->setEnabled(enabled);
}

bool FileNameIndexDBus::CreateIndexTask(const QStringList &paths, const QVariantMap &options)
{
    Q_UNUSED(options)
    return d->runtime->taskManager()->startTask(IndexTask::Type::Create, paths);
}

bool FileNameIndexDBus::UpdateIndexTask(const QStringList &paths, const QVariantMap &options)
{
    Q_UNUSED(options)
    return d->runtime->taskManager()->startTask(IndexTask::Type::Update, paths);
}

bool FileNameIndexDBus::StopCurrentTask()
{
    if (!d->runtime->taskManager()->hasRunningTask()) {
        return false;
    }

    d->runtime->taskManager()->stopCurrentTask();
    return true;
}

bool FileNameIndexDBus::HasRunningTask()
{
    return d->runtime->taskManager()->hasRunningTask();
}

bool FileNameIndexDBus::IndexDatabaseExists()
{
    if (!d->runtime->profile().isIndexAvailable()) {
        return false;
    }

    if (!d->runtime->stateStore().isCompatibleVersion()) {
        fmWarning() << "FileNameIndexDBus: Index database exists but version is incompatible."
                    << "Current version:" << d->runtime->profile().runtimeIndexVersion()
                    << "Stored version:" << d->runtime->stateStore().getIndexVersion();
        return false;
    }

    if (d->runtime->stateStore().getLastUpdateTime().isEmpty()) {
        fmWarning() << "FileNameIndexDBus: Last update time is empty, index may be corrupted";
        return false;
    }

    return true;
}

QString FileNameIndexDBus::GetLastUpdateTime()
{
    return d->runtime->stateStore().getLastUpdateTime();
}

bool FileNameIndexDBus::ProcessFileChanges(const QStringList &createdFiles,
                                      const QStringList &modifiedFiles,
                                      const QStringList &deletedFiles)
{
    bool tasksQueued = false;

    if (!deletedFiles.isEmpty()) {
        fmInfo() << "FileNameIndexDBus: Processing" << deletedFiles.size() << "deleted files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::RemoveFileList, deletedFiles) || tasksQueued;
    }

    if (!createdFiles.isEmpty()) {
        fmInfo() << "FileNameIndexDBus: Processing" << createdFiles.size() << "created files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::CreateFileList, createdFiles) || tasksQueued;
    }

    if (!modifiedFiles.isEmpty()) {
        fmInfo() << "FileNameIndexDBus: Processing" << modifiedFiles.size() << "modified files";
        tasksQueued = d->runtime->taskManager()->startFileListTask(IndexTask::Type::UpdateFileList, modifiedFiles) || tasksQueued;
    }

    return tasksQueued;
}

bool FileNameIndexDBus::ProcessFileMoves(const QHash<QString, QString> &movedFiles)
{
    if (movedFiles.isEmpty()) {
        fmDebug() << "FileNameIndexDBus: No file moves to process";
        return false;
    }

    fmInfo() << "FileNameIndexDBus: Processing" << movedFiles.size() << "moved files";
    return d->runtime->taskManager()->startFileMoveTask(movedFiles);
}

QVariantMap FileNameIndexDBus::GetIndexStatus()
{
    QVariantMap status;
    auto *tm = d->runtime->taskManager();

    // Always delegate to TaskManager::currentIndexStatus() so the client
    // receives the specific waiting state (WaitingPower / WaitingPowerSave /
    // WaitingIdle / WaitingUpgrade / Failed / Running / Idle) rather than
    // the generic "Blocked"/"Idle" that bypasses environment checks.
    status["state"] = tm->currentIndexStatus();
    auto grade = tm->currentOrQueuedGrade();
    status["grade"] = grade.has_value() ? TaskManager::gradeToString(*grade) : QStringLiteral("none");

    return status;
}

bool FileNameIndexDBus::ForceUpdateIndex(const QStringList &paths, const QVariantMap &options)
{
    const bool manual = options.value("manual", true).toBool();
    fmInfo() << "FileNameIndexDBus: Force update index requested for" << paths.size() << "paths"
             << "manual:" << manual;

    bool exists = IndexDatabaseExists();
    auto type = exists ? IndexTask::Type::Update : IndexTask::Type::Create;

    if (manual) {
        return d->runtime->taskManager()->startTask(type, paths, IndexTask::Grade::Manual, true);
    }
    return d->runtime->taskManager()->startTask(type, paths, IndexTask::Grade::None, true);
}
