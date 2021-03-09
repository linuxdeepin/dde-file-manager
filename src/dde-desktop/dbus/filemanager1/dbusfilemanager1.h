/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef DBUSFILEMANAGER1_H
#define DBUSFILEMANAGER1_H

#include <QObject>
#include <QDBusMessage>


class DBusFileManager1 : public QObject
{
    Q_OBJECT

public:
    explicit DBusFileManager1(QObject *parent = nullptr);

    void ShowFolders(const QStringList &URIs, const QString &StartupId);
    void ShowItemProperties(const QStringList &URIs, const QString &StartupId);
    void ShowItems(const QStringList &URIs, const QString &StartupId);
    void Trash(const QStringList &URIs);

    // debug function
    QStringList GetMonitorFiles() const;

signals:
    /**
     * @brief lockEventTriggered 锁屏信号
     * @param user
     */
    void lockEventTriggered(QString user);

public slots:
    // 保险箱使用
    bool topTaskDialog();
    void closeTask();

private slots:
    /**
     * @brief lockPropertyChanged 锁屏改变事件
     * @param msg
     */
    void lockPropertyChanged(const QDBusMessage &msg);
};

#endif // DBUSFILEMANAGER1_H
