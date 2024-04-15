// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkutils.h"

#include <QtConcurrent>
#include <QFutureWatcher>
#include <QTcpSocket>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace dfmbase;

NetworkUtils *NetworkUtils::instance()
{
    static NetworkUtils s;
    return &s;
}

bool NetworkUtils::checkNetConnection(const QString &host, const QString &port, int msecs)
{
    if (host.isEmpty())
        return true;

    QTcpSocket conn;
    conn.connectToHost(host, port.toInt());
    bool connected = conn.waitForConnected(msecs);
    qCInfo(logDFMBase) << "connect to host" << host
                       << "at port" << port
                       << "result:" << connected << conn.error();
    return connected;
}

void NetworkUtils::doAfterCheckNet(const QString &host, const QStringList &ports, std::function<void(bool)> callback, int msecs)
{
    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, [callback, watcher]() {
        if (callback)
            callback(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run([host, ports, msecs]() {
        for (const auto &port : ports) {
            qApp->processEvents();
            if (NetworkUtils::instance()->checkNetConnection(host, port, msecs))
                return true;
        }
        return false;
    }));
}

bool NetworkUtils::parseIp(const QString &mpt, QString &ip, QString &port)
{
    static constexpr char kSmbPort[] { "445" };
    static constexpr char kFtpPort[] { "21" };
    static constexpr char kSftpPort[] { "22" };

    QString s(mpt);
    static QRegularExpression gvfsPref { "(^/run/user/\\d+/gvfs/|^/root/\\.gvfs/)" };
    static QRegularExpression cifsMptPref { "^/media/[\\s\\S]*/smbmounts/" };   // TODO(xust) smb mount point may be changed.

    if (s.contains(gvfsPref))
        s.remove(gvfsPref);
    else if (s.contains(cifsMptPref))
        s.remove(cifsMptPref);
    else
        return false;

    // s = ftp:host=1.2.3.4  smb-share:server=1.2.3.4,share=draw
    bool isFtp = s.startsWith("ftp");
    bool isSftp = s.startsWith("sftp");
    bool isSmb = s.startsWith("smb");

    if (!isFtp && !isSftp && !isSmb)
        return false;

    // ftp:host=1.2.3.4,port=123  smb-share:port=321,server=1.2.3.4,share=draw
    static QRegularExpression hostAndPortRegx(R"(([:,]port=(?<port0>\d*))?[,:](server|host)=(?<host>[^/:,]+)(,port=(?<port1>\d*))?)");
    // --------------------------------------------------forward PORT ---|--------------------- ip/host ---|--- backward PORT ---|, PORT is optional
    auto match = hostAndPortRegx.match(s);
    if (match.hasMatch()) {
        auto capturedPort = match.captured("port0");
        if (capturedPort.isEmpty())
            capturedPort = match.captured("port1");

        if (!capturedPort.isEmpty())
            port = capturedPort;
        else if (isSmb)
            port = kSmbPort;
        else if (isFtp)
            port = kFtpPort;
        else if (isSftp)
            port = kSftpPort;
        else
            port = kSmbPort;

        ip = match.captured("host");
        return true;
    }

    return false;
}

bool NetworkUtils::checkFtpOrSmbBusy(const QUrl &url)
{
    QString host, port;
    if (!parseIp(url.path(), host, port))
        return false;

    auto busy = !checkNetConnection(host, port);
    if (busy)
        qCInfo(logDFMBase) << "can not connect url = " << url << " host =  " << host << " port = " << port;

    return busy;
}

NetworkUtils::NetworkUtils(QObject *parent)
    : QObject(parent)
{
}
