/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dengkeyun <dengkeyun@uniontech.com>
 *
 * Maintainer: dengkeyun <dengkeyun@uniontech.com>
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

#ifndef DEFENDER_INTERFACE_H
#define DEFENDER_INTERFACE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QDBusInterface>

class DefenderInterface : public QObject
{
    Q_OBJECT

public:
    DefenderInterface(QObject *parent = nullptr);
    ~DefenderInterface() {}

    /*
     * 检查当前扫描路径中是否存在url或url的子目录
     */
    bool isScanning(const QUrl &url);
    bool isScanning(const QList<QUrl> &urls);
    /*
     * 停止扫描url及url的子目录，返回false为停止失败
     */
    bool stopScanning(const QUrl &url);
    bool stopScanning(const QList<QUrl> &urls);

public Q_SLOTS:
    void scanningUsbPathsChanged(QStringList list);

private:
    // 从当前扫描路径选取url的子目录，若url本身也被扫描，也会在返回列表中
    QList<QUrl> getScanningPaths(const QUrl &url = QUrl("/"));
    void start();

private:
    QScopedPointer<QDBusInterface> interface;
    QList<QUrl> scanningPaths;
    bool m_started;
};

#endif // DEFENDER_INTERFACE_H
