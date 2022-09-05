// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

