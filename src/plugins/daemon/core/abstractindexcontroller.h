// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTINDEXCONTROLLER_H
#define ABSTRACTINDEXCONTROLLER_H

#include "daemonplugin_core_global.h"
#include "indexcontrollerdescriptor.h"

#include <QObject>
#include <QTimer>
#include <QScopedPointer>

class QDBusAbstractInterface;

DAEMONPCORE_BEGIN_NAMESPACE

class AbstractIndexController : public QObject
{
    Q_OBJECT

public:
    explicit AbstractIndexController(IndexControllerDescriptor descriptor, QObject *parent = nullptr);
    ~AbstractIndexController() override;

    void initialize();

protected:
    enum class State {
        Disabled,
        Idle,
        Running
    };

    QStringList indexedPaths() const;
    const IndexControllerDescriptor &descriptor() const;

private Q_SLOTS:
    void onTaskFinished(const QString &type, const QString &path, bool success);
    void onTaskProgressChanged(const QString &type, const QString &path, qint64 count, qint64 total);

private:
    using StateHandler = std::function<void(bool enable)>;
    using TaskFinishHandler = std::function<void(bool success)>;

    void setupStateHandlers();
    void setupDBusConnections();
    void startIndexTask(bool isCreate);
    void updateState(State newState);
    void handleConfigChanged(const QString &config, const QString &key);
    void activeBackend(bool isInit = false);
    void keepBackendAlive();
    bool isBackendAvaliable();
    void updateKeepAliveTimer();
    bool isTrackedPath(const QString &path) const;

private:
    IndexControllerDescriptor m_descriptor;
    QScopedPointer<QDBusAbstractInterface> interface;
    State currentState { State::Disabled };
    bool isConfigEnabled { false };
    QTimer *keepAliveTimer { nullptr };
    std::map<State, StateHandler> stateHandlers;
    std::map<State, TaskFinishHandler> taskFinishHandlers;
};

DAEMONPCORE_END_NAMESPACE

#endif   // ABSTRACTINDEXCONTROLLER_H
