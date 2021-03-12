/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#pragma once

#include <QMap>
#include <QString>
#include <QObject>


typedef QMap<QString, QMap<QString, QString>> DatabaseMap;

class DeviceInfoParser: public QObject
{
    Q_OBJECT

public:
    static DeviceInfoParser &Instance()
    {
        static DeviceInfoParser _instance;
        return _instance;
    }

    // 刷新硬件数据
    void refreshDabase();

    // 获取cdrom个数
    QStringList getLshwCDRomList();

    // cdrom是否是内部设备
    bool isInternalDevice(const QString &device);

signals:
    void loadFinished();

protected:
    const QString &queryData(const QString &toolname, const QString &firstKey, const QString &secondKey);
    bool loadLshwDatabase();
    bool executeProcess(const QString &cmd);

private:
    DeviceInfoParser();
    ~DeviceInfoParser();

    QString m_standOutput; // 存储命令输出

    QMap<QString, DatabaseMap> m_toolDatabase;
    QMap<QString, QStringList> m_toolDatabaseSecondOrder;
};

