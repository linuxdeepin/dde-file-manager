// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXCONTROLLER_H
#define TEXTINDEXCONTROLLER_H

#include "daemonplugin_core_global.h"

#include <QObject>
#include <QTimer>

#include <memory>
#include <functional>

class OrgDeepinFilemanagerTextIndexInterface;

DAEMONPCORE_BEGIN_NAMESPACE

class TextIndexController : public QObject
{
    Q_OBJECT
public:
    explicit TextIndexController(QObject *parent = nullptr);
    ~TextIndexController();

    void initialize();

private:
    enum class State {
        Disabled,   // 表示功能完全关闭的状态，不会进行任何索引操作
        Idle,   // 功能已启用但当前没有执行索引任务，处于等待状态
        Running   // 正在执行索引任务
    };

    using StateHandler = std::function<void(bool enable)>;
    using TaskFinishHandler = std::function<void(bool success)>;

    void setupDBusConnections();
    void startIndexTask(bool isCreate);
    void updateState(State newState);
    void handleConfigChanged(const QString &config, const QString &key);
    void activeBackend(bool isInit = false);
    void keepBackendAlive();
    bool isBackendAvaliable();
    void updateKeepAliveTimer();

private:
    std::unique_ptr<OrgDeepinFilemanagerTextIndexInterface> interface;
    State currentState { State::Disabled };
    bool isConfigEnabled { false };
    QTimer *keepAliveTimer { nullptr };

    // 状态处理器映射
    std::map<State, StateHandler> stateHandlers;
    std::map<State, TaskFinishHandler> taskFinishHandlers;
};

DAEMONPCORE_END_NAMESPACE

#endif   // TEXTINDEXCONTROLLER_H
