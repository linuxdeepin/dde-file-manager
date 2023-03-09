// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTEREVENTRECEIVER_H
#define COMPUTEREVENTRECEIVER_H

#include "dfmplugin_computer_global.h"

#include <QObject>

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

protected:
    bool parseCifsMountCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);

private:
    explicit ComputerEventReceiver(QObject *parent = nullptr);
};

}

#endif   // COMPUTEREVENTRECEIVER_H
