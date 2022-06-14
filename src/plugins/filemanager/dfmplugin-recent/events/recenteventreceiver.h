/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef RECENTEVENTRECEIVER_H
#define RECENTEVENTRECEIVER_H

#include "dfmplugin_recent_global.h"

#include <QObject>

DPRECENT_BEGIN_NAMESPACE

class RecentEventReceiver : public QObject
{
    Q_OBJECT

public:
    static RecentEventReceiver *instance();
    void initConnect();

public slots:
    void handleAddressInputStr(QString *str);
    void handleWindowUrlChanged(quint64 winId, const QUrl &url);

private:
    explicit RecentEventReceiver(QObject *parent = nullptr);
};

DPRECENT_END_NAMESPACE

#endif   // RECENTEVENTRECEIVER_H
