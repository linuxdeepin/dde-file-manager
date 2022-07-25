/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DISCDEVICESCANNER_H
#define DISCDEVICESCANNER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QTimer>
#include <QDBusVariant>

namespace dfmbase {

class DiscDeviceScanner final : public QObject
{
    Q_OBJECT

public:
    explicit DiscDeviceScanner(QObject *parent = nullptr);

    void initialize();

private:
    bool startScan();
    void stopScan();
    void updateScanState();

private slots:
    void scanOpticalDisc();
    void onDevicePropertyChangedQVar(const QString &id, const QString &propertyName, const QVariant &var);
    void onDevicePropertyChangedQDBusVar(const QString &id, const QString &propertyName, const QDBusVariant &var);
    void onDiscWoringStateChanged(const QString &id, const QString &dev, bool working);

private:
    QScopedPointer<QTimer> discScanTimer { new QTimer };
    QStringList discDevIdGroup;
};

}

#endif   // DISCDEVICESCANNER_H
