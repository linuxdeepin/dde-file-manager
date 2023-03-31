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

    bool checkNetConnection(const QString &host, const QString &port);
    void doAfterCheckNet(const QString &host, const QString &port, std::function<void(bool)> callback = nullptr);
    bool parseIp(const QString &mpt, QString &ip, QString &port);
    bool checkFtpOrSmbBusy(const QUrl &url);

protected:
    explicit NetworkUtils(QObject *parent = nullptr);
};

}

#endif   // NETWORKUTILS_H
