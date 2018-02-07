/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef USBFORMATTER_H
#define USBFORMATTER_H

#include <QObject>
#include "../partman/partitionmanager.h"

class UsbFormatterAdaptor;

class UsbFormatter : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit UsbFormatter(QObject *parent = 0);

signals:

public slots:
   bool mkfs(const QString& path, const QString& fs, const QString& label);

private:
    UsbFormatterAdaptor* m_usbFormatterAdaptor = NULL;
    PartMan::PartitionManager* m_partitionManager = NULL;
};

#endif // USBFORMATTER_H
