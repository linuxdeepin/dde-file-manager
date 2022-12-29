/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

private:
    explicit ComputerEventReceiver(QObject *parent = nullptr);
};

}

#endif   // COMPUTEREVENTRECEIVER_H
