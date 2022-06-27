/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DEFENDERHELPER_H
#define DEFENDERHELPER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QUrl>
#include <QDBusInterface>

#include <mutex>

namespace dfmbase {

class DefenderController : public QObject
{
    friend class DeviceHelper;
    Q_OBJECT
    Q_DISABLE_COPY(DefenderController)

private:
    static DefenderController &instance();

    bool isScanning(const QUrl &url);
    bool isScanning(const QList<QUrl> &urls);
    bool stopScanning(const QUrl &url);
    bool stopScanning(const QList<QUrl> &urls);

    Q_SLOT void scanningUsbPathsChanged(const QStringList &list);

private:
    QList<QUrl> getScanningPaths(const QUrl &url = QUrl("/"));
    void start();

private:
    explicit DefenderController(QObject *parent = nullptr);
    ~DefenderController();
    static std::once_flag &onceFlag();

private:
    QScopedPointer<QDBusInterface> interface;
    QList<QUrl> scanningPaths;
};

}

#define DefenderInstance DefenderController::instance()

#endif   // DEFENDERHELPER_H
