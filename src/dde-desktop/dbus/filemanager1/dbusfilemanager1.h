// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
