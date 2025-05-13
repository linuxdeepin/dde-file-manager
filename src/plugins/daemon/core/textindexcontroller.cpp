// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
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

        if (!isBackendAvaliable()) {
            fmInfo() << "[TextIndex] Setting up DBus connections in Idle state";
            setupDBusConnections();
        }

        fmInfo() << "[TextIndex] Checking index database existence";
        QDBusPendingCall pendingCall = interface->IndexDatabaseExists();
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *call) {
            QDBusPendingReply<bool> reply = *watcher;

            if (reply.isError()) {
                fmWarning() << "[TextIndex] Failed to check index existence:" << reply.error().message();
            } else {
                bool needCreate = !reply.value();
                fmInfo() << "[TextIndex] Index check result - Need create:" << needCreate;
                startIndexTask(needCreate);
            }

            watcher->deleteLater();
        });
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
            isConfigEnabled = false;
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
    isConfigEnabled = DConfigManager::instance()->value(kSearchCfgPath, kEnableFullTextSearch).toBool();
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
        fmInfo() << "[TextIndex] Full text search enable changed:" << isConfigEnabled << "->" << newEnabled;
        isConfigEnabled = newEnabled;
        activeBackend();
        updateKeepAliveTimer();

        if (auto handler = stateHandlers.find(currentState); handler != stateHandlers.end()) {
            fmInfo() << "[TextIndex] Triggering state handler for current state:" << static_cast<int>(currentState);
            handler->second(isConfigEnabled);
        } else {
            fmWarning() << "[TextIndex] No handler found for current state:" << static_cast<int>(currentState);
        }
    }
}

void TextIndexController::activeBackend(bool isInit)
{
    if (!isBackendAvaliable()) {
        fmWarning() << "activeBackend failed, DBus interface not initialized";
        return;
    }

    if (isInit) {
        fmInfo() << "Init backend";
        interface->Init();
    }
    fmInfo() << "Active backend: " << isConfigEnabled;
    interface->SetEnabled(isConfigEnabled);
}

void TextIndexController::keepBackendAlive()
{
    if (!isBackendAvaliable()) {
        fmWarning() << "keepBackend failed, DBus interface not initialized";
        return;
    }

    bool backendEnabled = interface->IsEnabled();
    fmInfo() << "Textindex backend status: " << backendEnabled;

    // 配置启动全文索引时，backendEnabled 一定要 true，
    // 若不满足说明 backend 出现了崩溃等异常，需要重新激活
    if (!backendEnabled && isConfigEnabled)
        activeBackend();
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
    if (isConfigEnabled && !keepAliveTimer->isActive())
        keepAliveTimer->start();
    else if (!isConfigEnabled && keepAliveTimer->isActive())
        keepAliveTimer->stop();
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
    if (isCreate) {
        fmInfo() << "[TextIndex] Starting CREATE task for root directory";
        pendingTask = interface->CreateIndexTask(DFMSEARCH::Global::defaultIndexedDirectory());
    } else {
        fmInfo() << "[TextIndex] Starting UPDATE task for root directory";
        pendingTask = interface->UpdateIndexTask(DFMSEARCH::Global::defaultIndexedDirectory());
    }

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingTask, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            fmWarning() << "[TextIndex] Failed to start task:" << reply.error().message();
        } else if (reply.value()) {
            fmInfo() << "[TextIndex] Task started successfully, transitioning to Running state";
            updateState(State::Running);
        } else {
            fmWarning() << "[TextIndex] Task start returned false";
        }
        watcher->deleteLater();   // 清理 watcher
    });
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

DAEMONPCORE_END_NAMESPACE
