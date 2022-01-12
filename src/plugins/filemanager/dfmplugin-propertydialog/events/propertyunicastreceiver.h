/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PROPERTYDIALOGUNICASTRECEIVER_H
#define PROPERTYDIALOGUNICASTRECEIVER_H

#include "dfmplugin_propertydialog_global.h"
#include "services/common/propertydialog/property_defines.h"

#include <QObject>

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class PropertyUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyUnicastReceiver)
private:
    explicit PropertyUnicastReceiver(QObject *parent = nullptr);

public:
    static PropertyUnicastReceiver *instance();
    void connectService();

signals:

public slots:
    void addFilePropertyControl(const QList<QUrl> &url);

    void addDeviceProperty(const DSC_NAMESPACE::DeviceInfo &info);

    void showTrashProperty(const QUrl &url);

    void showComputerProperty(const QUrl &url);
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // PROPERTYDIALOGEVENTRECEIVER_H
