// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "private/textindexdbus_p.h"
#include "utils/indexutility.h"

static constexpr char kTextIndexObjPath[] { "/org/deepin/Filemanager/TextIndex" };

SERVICETEXTINDEX_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(SERVICETEXTINDEX_NAMESPACE)
SERVICETEXTINDEX_END_NAMESPACE

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

void TextIndexDBusPrivate::initialize()
{
    fsEventController->setupFSEventCollector();
}

void TextIndexDBusPrivate::initConnect()
{
    QObject::connect(taskManager, &TaskManager::taskFinished,
                     q, [this](const QString &type, const QString &path, bool success) {
                         emit q->TaskFinished(type, path, success);
                     });

    QObject::connect(taskManager, &TaskManager::taskProgressChanged,
                     q, [this](const QString &type, const QString &path, qint64 count, qint64 total) {
                         emit q->TaskProgressChanged(type, path, count, total);
                     });

    QObject::connect(fsEventController, &FSEventController::requestProcessFileChanges,
                     q, &TextIndexDBus::ProcessFileChanges);
    QObject::connect(fsEventController, &FSEventController::monitoring,
                     q, [this](bool start) {
                         handleMonitoring(start);
                     });
    QObject::connect(fsEventController, &FSEventController::requestSlientStart,
                     q, [this]() {
                         handleSlientStart();
                     });
}

void TextIndexDBusPrivate::handleMonitoring(bool start)
{
    fmInfo() << "FS event monitoring: " << start;
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
        // Create or update index es silently
        const auto &dirs = DFMSEARCH::Global::defaultIndexedDirectory();
        const auto &path = dirs.isEmpty() ? QDir::homePath() : dirs.first();

        if (!canSilentlyRefreshIndex(path)) {
            fmWarning() << "Unable to refresh the index because there is already a current task for: " << path;
            return;
        }

        fmInfo() << "Start a task silently for: " << path;
        if (q->IndexDatabaseExists()) {   // update
            q->UpdateIndexTask(path);
        } else {   // create
            q->CreateIndexTask(path);
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

bool TextIndexDBus::CreateIndexTask(const QString &path)
{
    return d->taskManager->startTask(IndexTask::Type::Create, path);
}

bool TextIndexDBus::UpdateIndexTask(const QString &path)
{
    return d->taskManager->startTask(IndexTask::Type::Update, path);
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
        fmWarning() << "Index database exists but version is incompatible or missing."
                    << "Current version:" << Defines::kIndexVersion
                    << "Stored version:" << IndexUtility::getIndexVersion()
                    << "[Index considered invalid due to version mismatch]";
        return false;
    }

    if (IndexUtility::getLastUpdateTime().isEmpty()) {
        fmWarning() << "Last update time is empty";
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
        fmInfo() << "Processing" << deletedFiles.size() << "deleted files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::RemoveFileList, deletedFiles) || tasksQueued;
    }

    // 处理新增的文件
    if (!createdFiles.isEmpty()) {
        fmInfo() << "Processing" << createdFiles.size() << "created files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::CreateFileList, createdFiles) || tasksQueued;
    }

    // 处理修改的文件
    if (!modifiedFiles.isEmpty()) {
        fmInfo() << "Processing" << modifiedFiles.size() << "modified files";
        tasksQueued = d->taskManager->startFileListTask(IndexTask::Type::UpdateFileList, modifiedFiles) || tasksQueued;
    }

    return tasksQueued;
}
