// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERHELPER_H
#define DEFENDERHELPER_H

#include <dfm-base/dfm_base_global.h>

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

private:
    QScopedPointer<QDBusInterface> interface;
    QList<QUrl> scanningPaths;
};

}

#define DefenderInstance DefenderController::instance()

#endif   // DEFENDERHELPER_H
