// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexcontroller.h"

#include "textindex_interface.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
DAEMONPCORE_BEGIN_NAMESPACE

static constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
static constexpr char kEnableFullTextSearch[] { "enableFullTextSearch" };

TextIndexController::TextIndexController(QObject *parent)
    : QObject(parent)
{
    // 初始化状态处理器
    stateHandlers[State::Disabled] = [this](bool enable) {
        fmInfo() << "[TextIndex] Disabled state handler called with enable:" << enable;
        if (enable) {
            updateState(State::Idle);
            fmInfo() << "[TextIndex] Transitioning from Disabled to Idle, triggering Idle handler";
            stateHandlers[State::Idle](true);
        }
    };

    stateHandlers[State::Idle] = [this](bool enable) {
        fmInfo() << "[TextIndex] Idle state handler called with enable:" << enable;
        if (!enable) {
            fmInfo() << "[TextIndex] Service disabled, transitioning to Disabled state";
            updateState(State::Disabled);
            return;
        }

        if (!interface) {
            fmInfo() << "[TextIndex] Setting up DBus connections in Idle state";
            setupDBusConnections();
        }

        // Start FS monitoring when idle state is activated
        startFSMonitoring();

        fmInfo() << "[TextIndex] Checking index database existence";
        auto pendingExists = interface->IndexDatabaseExists();
        pendingExists.waitForFinished();
        auto lastUpdateTime = interface->GetLastUpdateTime();
        lastUpdateTime.waitForFinished();

        if (pendingExists.isError() || lastUpdateTime.isError()) {
            fmWarning() << "[TextIndex] Failed to check index existence:" << pendingExists.error().message();
            return;
        }

        bool needCreate = (!pendingExists.value() || lastUpdateTime.value().isEmpty());
        fmInfo() << "[TextIndex] Index check result - Need create:" << needCreate;
        startIndexTask(needCreate);
    };

    stateHandlers[State::Running] = [this](bool enable) {
        fmInfo() << "[TextIndex] Running state handler called with enable:" << enable;
        if (!enable) {
            if (interface) {
                fmInfo() << "[TextIndex] Stopping current task due to disable request";
                interface->StopCurrentTask();
            }
            updateState(State::Disabled);
        }
    };

    taskFinishHandlers[State::Running] = [this](bool success) {
        fmInfo() << "[TextIndex] Task finished handler called with success:" << success;
        if (success) {
            fmInfo() << "[TextIndex] Task completed successfully, transitioning to Idle state";
            updateState(State::Idle);
        } else {
            fmWarning() << "[TextIndex] Task failed, transitioning to Disabled state";
            updateState(State::Disabled);
            isEnabled = false;
            fmWarning() << "[TextIndex] Service disabled due to task failure";
        }
    };
}

TextIndexController::~TextIndexController() = default;

void TextIndexController::initialize()
{
    fmInfo() << "[TextIndex] Initializing controller";
    QString err;
    auto ret = DConfigManager::instance()->addConfig(kSearchCfgPath, &err);
    if (!ret) {
        fmWarning() << "[TextIndex] Failed to register search config:" << err;
        return;
    }
    fmInfo() << "[TextIndex] Successfully registered search config";
    isEnabled = DConfigManager::instance()->value(kSearchCfgPath, kEnableFullTextSearch).toBool();

    // Setup FSEventCollector
    setupFSEventCollector();

    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            this, &TextIndexController::handleConfigChanged);
}

void TextIndexController::handleConfigChanged(const QString &config, const QString &key)
{
    if (config == kSearchCfgPath && key == kEnableFullTextSearch) {
        bool newEnabled = DConfigManager::instance()->value(config, key).toBool();
        fmInfo() << "[TextIndex] Full text search enable changed:" << isEnabled << "->" << newEnabled;
        isEnabled = newEnabled;

        if (!isEnabled)
            stopFSMonitoring();

        if (auto handler = stateHandlers.find(currentState); handler != stateHandlers.end()) {
            fmInfo() << "[TextIndex] Triggering state handler for current state:" << static_cast<int>(currentState);
            handler->second(isEnabled);
        } else {
            fmWarning() << "[TextIndex] No handler found for current state:" << static_cast<int>(currentState);
        }
    }
}

void TextIndexController::setupDBusConnections()
{
    // 先确保服务已启动
    QDBusConnectionInterface *sessionBusIface = QDBusConnection::sessionBus().interface();
    if (sessionBusIface)
        sessionBusIface->startService("org.deepin.Filemanager.TextIndex");
    interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
            "org.deepin.Filemanager.TextIndex",
            "/org/deepin/Filemanager/TextIndex",
            QDBusConnection::sessionBus(),
            this));

    connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskFinished,
            this, [this](const QString &type, const QString &path, bool success) {
                if (path != QDir::homePath())
                    return;
                if (auto handler = taskFinishHandlers.find(currentState); handler != taskFinishHandlers.end()) {
                    handler->second(success);
                }
            });
}

void TextIndexController::startIndexTask(bool isCreate)
{
    if (!interface) {
        fmWarning() << "[TextIndex] Cannot start task: DBus interface not initialized";
        return;
    }

    QDBusPendingReply<bool> pendingTask;
    // TODO(search): dfm-search
    if (isCreate) {
        fmInfo() << "[TextIndex] Starting CREATE task for root directory";
        pendingTask = interface->CreateIndexTask(QDir::homePath());
    } else {
        fmInfo() << "[TextIndex] Starting UPDATE task for root directory";
        pendingTask = interface->UpdateIndexTask(QDir::homePath());
    }

    pendingTask.waitForFinished();
    if (pendingTask.isError()) {
        fmWarning() << "[TextIndex] Failed to start task:" << pendingTask.error().message();
    } else if (pendingTask.value()) {
        fmInfo() << "[TextIndex] Task started successfully, transitioning to Running state";
        updateState(State::Running);
    } else {
        fmWarning() << "[TextIndex] Task start returned false";
    }
}

void TextIndexController::updateState(State newState)
{
    fmInfo() << "[TextIndex] State transition:"
             << "Disabled(" << static_cast<int>(State::Disabled) << ")"
             << "Idle(" << static_cast<int>(State::Idle) << ")"
             << "Running(" << static_cast<int>(State::Running) << ")"
             << static_cast<int>(currentState) << "->" << static_cast<int>(newState);
    currentState = newState;
}

// FSEventCollector setup and handling methods

void TextIndexController::setupFSEventCollector()
{
    fsEventCollector = std::make_unique<FSEventCollector>(this);
    fsEventCollector->setCollectionInterval(60);   // Default 1 minutes
    fsEventCollector->setMaxEventCount(10000);   // Default 10k events
    if (isEnabled)
        startFSMonitoring();
    connect(fsEventCollector.get(), &FSEventCollector::filesCreated,
            this, &TextIndexController::onFilesCreated);
    connect(fsEventCollector.get(), &FSEventCollector::filesDeleted,
            this, &TextIndexController::onFilesDeleted);
    connect(fsEventCollector.get(), &FSEventCollector::filesModified,
            this, &TextIndexController::onFilesModified);
    connect(fsEventCollector.get(), &FSEventCollector::flushFinished,
            this, &TextIndexController::onFlushFinished);
}

void TextIndexController::startFSMonitoring()
{
    if (!fsEventCollector) {
        fmWarning() << "[TextIndex] Cannot start FS monitoring: FSEventCollector not initialized";
        return;
    }

    if (fsEventCollector->isActive()) {
        fmInfo() << "[TextIndex] FS monitoring already active";
        return;
    }

    // Initialize with home directory (same as text index)
    bool success = fsEventCollector->initialize(QDir::homePath());
    if (!success) {
        fmWarning() << "[TextIndex] Failed to initialize FSEventCollector";
        return;
    }

    // Clear any previously collected events
    collectedCreatedFiles.clear();
    collectedDeletedFiles.clear();
    collectedModifiedFiles.clear();

    // Start the collector
    success = fsEventCollector->start();
    if (!success) {
        fmWarning() << "[TextIndex] Failed to start FSEventCollector";
        return;
    }

    fmInfo() << "[TextIndex] FS monitoring started successfully";
}

void TextIndexController::stopFSMonitoring()
{
    if (!fsEventCollector || !fsEventCollector->isActive()) {
        return;
    }

    fsEventCollector->stop();

    // Clear any collected events
    collectedCreatedFiles.clear();
    collectedDeletedFiles.clear();
    collectedModifiedFiles.clear();

    fmInfo() << "[TextIndex] FS monitoring stopped";
}

void TextIndexController::processFSEvents()
{
    if (!interface) {
        fmWarning() << "[TextIndex] Cannot process FS events: DBus interface not initialized";
        setupDBusConnections();
    }
    Q_ASSERT(interface);
    // Check if we have any events to process
    if (collectedCreatedFiles.isEmpty() && collectedModifiedFiles.isEmpty() && collectedDeletedFiles.isEmpty()) {
        fmInfo() << "[TextIndex] No file system events to process";
        return;
    }

    fmInfo() << "[TextIndex] Processing file changes - Created:" << collectedCreatedFiles.size()
             << "Modified:" << collectedModifiedFiles.size()
             << "Deleted:" << collectedDeletedFiles.size();

    auto pendingTask = interface->ProcessFileChanges(
            collectedCreatedFiles, collectedModifiedFiles, collectedDeletedFiles);

    pendingTask.waitForFinished();
    if (pendingTask.isError()) {
        fmWarning() << "[TextIndex] Failed to process file changes:" << pendingTask.error().message();
    } else if (pendingTask.value()) {
        fmInfo() << "[TextIndex] File changes processing started successfully";
    } else {
        fmWarning() << "[TextIndex] File changes processing returned false";
    }

    // Clear collections
    collectedCreatedFiles.clear();
    collectedDeletedFiles.clear();
    collectedModifiedFiles.clear();
}

// FSEventCollector slots

void TextIndexController::onFilesCreated(const QStringList &paths)
{
    if (!isEnabled) {
        return;
    }

    fmInfo() << "[TextIndex] FS event: Files created -" << paths.size() << "items";
    collectedCreatedFiles.append(paths);
}

void TextIndexController::onFilesDeleted(const QStringList &paths)
{
    if (!isEnabled) {
        return;
    }

    fmInfo() << "[TextIndex] FS event: Files deleted -" << paths.size() << "items";
    collectedDeletedFiles.append(paths);
}

void TextIndexController::onFilesModified(const QStringList &paths)
{
    if (!isEnabled) {
        return;
    }

    fmInfo() << "[TextIndex] FS event: Files modified -" << paths.size() << "items";
    collectedModifiedFiles.append(paths);
}

void TextIndexController::onFlushFinished()
{
    if (!isEnabled) {
        return;
    }

    fmInfo() << "[TextIndex] FS event: Flush finished, processing events";
    processFSEvents();
}

DAEMONPCORE_END_NAMESPACE
