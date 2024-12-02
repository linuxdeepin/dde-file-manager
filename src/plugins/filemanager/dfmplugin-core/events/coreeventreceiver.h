// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREEVENTRECEIVER_H
#define COREEVENTRECEIVER_H

#include "dfmplugin_core_global.h"

#include <QObject>

DPCORE_BEGIN_NAMESPACE

class CoreEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CoreEventReceiver)

public:
    static CoreEventReceiver *instance();

public slots:
    void handleChangeUrl(quint64 windowId, const QUrl &url);
    void handleOpenWindow(const QUrl &url);
    void handleOpenWindow(const QUrl &url, const QVariant &opt);
    void handleLoadPlugins(const QStringList &names);
    void handleHeadless();
    void handleShowSettingDialog(quint64 windowId);

private:
    explicit CoreEventReceiver(QObject *parent = nullptr);
};

DPCORE_END_NAMESPACE

#endif   // COREEVENTRECEIVER_H
