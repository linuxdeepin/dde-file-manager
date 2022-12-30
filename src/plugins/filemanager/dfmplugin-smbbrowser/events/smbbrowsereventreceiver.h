/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu <zhuangshu@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef SMBBROWSEREVENTRECEIVER_H
#define SMBBROWSEREVENTRECEIVER_H

#include "dfmplugin_smbbrowser_global.h"

#include <QObject>

namespace dfmplugin_smbbrowser {

class SmbBrowserEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SmbBrowserEventReceiver)

public:
    static SmbBrowserEventReceiver *instance();

public Q_SLOTS:
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool cancelDelete(quint64, const QList<QUrl> &urls);

private:
    explicit SmbBrowserEventReceiver(QObject *parent = nullptr);
};

}

#endif   // SMBBROWSEREVENTRECEIVER_H
