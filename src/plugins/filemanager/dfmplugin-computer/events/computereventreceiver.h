// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTEREVENTRECEIVER_H
#define COMPUTEREVENTRECEIVER_H

#include "dfmplugin_computer_global.h"

#include <QObject>

#include <functional>

#define ComputerEventReceiverIns DPCOMPUTER_NAMESPACE::ComputerEventReceiver::instance()

namespace dfmplugin_computer {

class ComputerEventReceiver final : public QObject
{
    Q_OBJECT
public:
    static ComputerEventReceiver *instance();

public Q_SLOTS:
    void handleItemEject(const QUrl &url);
    bool handleSepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool handleSortItem(const QString &group, const QString &subGroup, const QUrl &a, const QUrl &b);
    bool handleSetTabName(const QUrl &url, QString *tabName);
    void setContextMenuEnable(bool enable);

    static void dirAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after);

protected:
    bool parseCifsMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool parseDevMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    static bool askForConfirmChmod(const QString &devName);

private:
    explicit ComputerEventReceiver(QObject *parent = nullptr);
};

}

#endif   // COMPUTEREVENTRECEIVER_H
