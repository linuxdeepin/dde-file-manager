// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBAREVENTRECEIVER_H
#define TITLEBAREVENTRECEIVER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>

namespace dfmplugin_titlebar {

class TitleBarEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TitleBarEventReceiver)

public:
    static TitleBarEventReceiver *instance();

public slots:
    // self slots
    bool handleCustomRegister(const QString &scheme, const QVariantMap &properties);

    void handleStartSpinner(quint64 windowId);
    void handleStopSpinner(quint64 windowId);

    void handleShowFilterButton(quint64 windowId, bool visible);
    void handleViewModeChanged(quint64 windowId, int mode);

    void handleSetNewWindowAndTabEnable(bool enable);
    void handleWindowForward(quint64 windowId);
    void handleWindowBackward(quint64 windowId);
    void handleRemoveHistory(quint64 windowId, const QUrl &url);

    bool handleTabAddable(quint64 windowId);
    void handleCloseTabs(const QUrl &url);
    void handleSetTabAlias(const QUrl &url, const QString &name);
    void handleOpenNewTabTriggered(quint64 windowId, const QUrl &url);

private:
    explicit TitleBarEventReceiver(QObject *parent = nullptr);
};

}

#endif   // TITLEBAREVENTRECEIVER_H
