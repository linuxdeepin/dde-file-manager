// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QString>

#include <functional>

namespace dfmbase {

class NetworkUtils : public QObject
{
    Q_OBJECT

public:
    static NetworkUtils *instance();

    bool checkNetConnection(const QString &host, const QString &port, int msecs = 1000);
    bool checkNetConnection(const QString &host, QStringList ports, int msecs = 1000);
    void doAfterCheckNet(const QString &host, const QStringList &ports,
                         std::function<void(bool)> callback = nullptr, int msecs = 3000);
    bool parseIp(const QString &mpt, QString &ip, QString &port);
    bool parseIp(const QString &mpt, QString &ip, QStringList &ports);
    bool checkFtpOrSmbBusy(const QUrl &url);
    // if network mount，get network mount
    static QMap<QString, QString> cifsMountHostInfo();

protected:
    explicit NetworkUtils(QObject *parent = nullptr);
};

}

#endif   // NETWORKUTILS_H
