// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexcontroller.h"

#include "textindex_interface.h"

#include <dfm-search/dsearch_global.h>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DFMBASE_USE_NAMESPACE
DAEMONPCORE_BEGIN_NAMESPACE

static constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
static constexpr char kEnableFullTextSearch[] { "enableFullTextSearch" };

TextIndexController::TextIndexController(QObject *parent)
    : QObject(parent), keepAliveTimer(new QTimer(this))
{
    // 初始化状态处理器
    stateHandlers[State::Disabled] = [this](bool enable) {
        fmDebug() << "[TextIndexController] Processing Disabled state with enable flag:" << enable;
        if (enable) {
            updateState(State::Idle);
            fmInfo() << "[TextIndexController] State transition: Disabled -> Idle, triggering Idle handler";
            stateHandlers[State::Idle](true);
        }
    };

    stateHandlers[State::Idle] = [this](bool enable) {
        fmDebug() << "[TextIndexController] Processing Idle state with enable flag:" << enable;
        if (!enable) {
            fmInfo() << "[TextIndexController] Service disabled, transitioning to Disabled state";
            updateState(State::Disabled);
            return;
        }

        if (!isBackendAvaliable()) {
            fmDebug() << "[TextIndexController] Setting up DBus connections in Idle state";
            setupDBusConnections();
        }

        fmDebug() << "[TextIndexController] Checking index database existence";
        QDBusPendingCall pendingCall = interface->IndexDatabaseExists();
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *call) {
            QDBusPendingReply<bool> reply = *watcher;

            if (reply.isError()) {
                fmWarning() << "[TextIndexController] Failed to check index database existence:" << reply.error().message();
            } else {
                bool needCreate = !reply.value();
                fmDebug() << "[TextIndexController] Index database check completed, need create:" << needCreate;
                startIndexTask(needCreate);
            }

            watcher->deleteLater();
        });
    };

    stateHandlers[State::Running] = [this](bool enable) {
        fmDebug() << "[TextIndexController] Processing Running state with enable flag:" << enable;
        if (!enable) {
            if (interface) {
                fmInfo() << "[TextIndexController] Stopping current task due to service disable request";
                interface->StopCurrentTask();
            }
            updateState(State::Disabled);
        }
    };

    taskFinishHandlers[State::Running] = [this](bool success) {
        fmDebug() << "[TextIndexController] Task finished with success:" << success;
        if (success) {
            fmInfo() << "[TextIndexController] Index task completed successfully, transitioning to Idle state";
            updateState(State::Idle);
        } else {
            fmWarning() << "[TextIndexController] Index task failed, disabling service";
            updateState(State::Disabled);
        }
    };
}

TextIndexController::~TextIndexController() = default;

void TextIndexController::initialize()
{
    fmInfo() << "[TextIndexController] Initializing text index controller";
    QString err;
    auto ret = DConfigManager::instance()->addConfig(kSearchCfgPath, &err);
    if (!ret) {
        fmWarning() << "[TextIndexController] Failed to register search config:" << err;
        return;
    }

    isConfigEnabled = DConfigManager::instance()->value(kSearchCfgPath, kEnableFullTextSearch).toBool();
    fmInfo() << "[TextIndexController] Search config registered successfully, full text search enabled:" << isConfigEnabled;

    keepAliveTimer->setInterval(5 * 60 * 1000);   // 5 min
    updateKeepAliveTimer();

    if (isConfigEnabled)
        activeBackend(true);   // init

    // 使用心跳，否则 textindex backend 会被退出
    connect(keepAliveTimer, &QTimer::timeout, this, &TextIndexController::keepBackendAlive);

    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            this, &TextIndexController::handleConfigChanged);
}

void TextIndexController::handleConfigChanged(const QString &config, const QString &key)
{
    if (config == kSearchCfgPath && key == kEnableFullTextSearch) {
        bool newEnabled = DConfigManager::instance()->value(config, key).toBool();
        fmInfo() << "[TextIndexController] Full text search configuration changed from" << isConfigEnabled << "to" << newEnabled;
        isConfigEnabled = newEnabled;
        activeBackend();
        updateKeepAliveTimer();

        if (auto handler = stateHandlers.find(currentState); handler != stateHandlers.end()) {
            fmDebug() << "[TextIndexController] Triggering state handler for current state:" << static_cast<int>(currentState);
            handler->second(isConfigEnabled);
        } else {
            fmWarning() << "[TextIndexController] No handler found for current state:" << static_cast<int>(currentState);
        }
    }
}

void TextIndexController::activeBackend(bool isInit)
{
    if (!isBackendAvaliable()) {
        fmWarning() << "[TextIndexController] Cannot activate backend, DBus interface not available";
        return;
    }

    if (isInit) {
        fmInfo() << "[TextIndexController] Initializing text index backend";
        interface->Init();
    }
    fmDebug() << "[TextIndexController] Setting backend enabled state to:" << isConfigEnabled;
    interface->SetEnabled(isConfigEnabled);
}

void TextIndexController::keepBackendAlive()
{
    if (!isBackendAvaliable()) {
        fmWarning() << "[TextIndexController] Cannot check backend status, DBus interface not available";
        return;
    }

    bool backendEnabled = interface->IsEnabled();
    fmDebug() << "[TextIndexController] Backend status check - enabled:" << backendEnabled << "config enabled:" << isConfigEnabled;

    // 配置启动全文索引时，backendEnabled 一定要 true，
    // 若不满足说明 backend 出现了崩溃等异常，需要重新激活
    if (!backendEnabled && isConfigEnabled) {
        fmWarning() << "[TextIndexController] Backend is disabled but config requires it enabled, reactivating backend";
        activeBackend();
    }
}

bool TextIndexController::isBackendAvaliable()
{
    if (!interface)
        setupDBusConnections();

    if (!interface)
        return false;

    return true;
}

void TextIndexController::updateKeepAliveTimer()
{
    if (isConfigEnabled && !keepAliveTimer->isActive()) {
        keepAliveTimer->start();
        fmDebug() << "[TextIndexController] Keep-alive timer started";
    } else if (!isConfigEnabled && keepAliveTimer->isActive()) {
        keepAliveTimer->stop();
        fmDebug() << "[TextIndexController] Keep-alive timer stopped";
    }
}

void TextIndexController::setupDBusConnections()
{
    fmDebug() << "[TextIndexController] Setting up DBus connections to text index service";

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
                fmDebug() << "[TextIndexController] Received TaskFinished signal - type:" << type << "path:" << path << "success:" << success;
                if (auto handler = taskFinishHandlers.find(currentState); handler != taskFinishHandlers.end()) {
                    handler->second(success);
                }
            });
    connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskProgressChanged,
            this, [this]() {
                if (currentState == State::Running)
                    return;
                fmDebug() << "[TextIndexController] Task progress changed, transitioning to Running state";
                updateState(State::Running);
                if (!isConfigEnabled) {
                    fmInfo() << "[TextIndexController] Service disabled during task execution, stopping task";
                    interface->StopCurrentTask();
                }
            });

    fmInfo() << "[TextIndexController] DBus connections established successfully";
}

void TextIndexController::startIndexTask(bool isCreate)
{
    if (!interface) {
        fmWarning() << "[TextIndexController] Cannot start index task, DBus interface not available";
        return;
    }

    QDBusPendingReply<bool> pendingTask;
    if (isCreate) {
        fmInfo() << "[TextIndexController] Starting CREATE index task for directory:" << DFMSEARCH::Global::defaultIndexedDirectory();
        pendingTask = interface->CreateIndexTask(DFMSEARCH::Global::defaultIndexedDirectory());
    } else {
        fmInfo() << "[TextIndexController] Starting UPDATE index task for directory:" << DFMSEARCH::Global::defaultIndexedDirectory();
        pendingTask = interface->UpdateIndexTask(DFMSEARCH::Global::defaultIndexedDirectory());
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingTask, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            fmWarning() << "[TextIndexController] Failed to start index task:" << reply.error().message();
        } else if (reply.value()) {
            fmInfo() << "[TextIndexController] Index task started successfully, transitioning to Running state";
            updateState(State::Running);
        } else {
            fmWarning() << "[TextIndexController] Index task start request returned false";
        }
        watcher->deleteLater();   // 清理 watcher
    });
}

void TextIndexController::updateState(State newState)
{
    if (currentState != newState) {
        fmInfo() << "[TextIndexController] State transition:"
                 << "Disabled(" << static_cast<int>(State::Disabled) << ")"
                 << "Idle(" << static_cast<int>(State::Idle) << ")"
                 << "Running(" << static_cast<int>(State::Running) << ")"
                 << static_cast<int>(currentState) << "->" << static_cast<int>(newState);
        currentState = newState;
    }
}

DAEMONPCORE_END_NAMESPACE
