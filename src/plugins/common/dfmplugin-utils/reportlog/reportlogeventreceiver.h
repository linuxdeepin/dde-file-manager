/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
 *
 * Maintainer: zhuangshu<zhuangshu@uniontech.com>
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
#ifndef REPORTLOGEVENTRECEIVER_H
#define REPORTLOGEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <dfm-mount/base/dmount_global.h>

#include <QObject>

namespace dfmplugin_utils {
class ReportLogEventReceiver : public QObject
{
    Q_OBJECT
public:
    explicit ReportLogEventReceiver(QObject *parent = nullptr);

    void bindEvents();

    void commit(const QString &type, const QVariantMap &args);
    void handleMenuData(const QString &name, const QList<QUrl> &urlList);

public Q_SLOTS:
    void handleMountNetworkResult(bool ret, DFMMOUNT::DeviceError err, const QString &msg);
};
}
#endif   // REPORTLOGEVENTRECEIVER_H
