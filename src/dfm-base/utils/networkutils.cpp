/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include "networkutils.h"

#include <QtConcurrent>
#include <QFutureWatcher>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace dfmbase;

NetworkUtils *NetworkUtils::instance()
{
    static NetworkUtils s;
    return &s;
}

bool NetworkUtils::checkNetConnection(const QString &host, const QString &port)
{
    if (host.isEmpty())
        return true;

    addrinfo *result;
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;   // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    char addressString[INET6_ADDRSTRLEN];
    const char *retval = nullptr;
    if (0 != getaddrinfo(host.toUtf8().toStdString().c_str(), port.toUtf8().toStdString().c_str(), &hints, &result)) {
        return false;
    }
    for (addrinfo *addr = result; addr != nullptr; addr = addr->ai_next) {
        int handle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (handle == -1) {
            continue;
        }
        if (::connect(handle, addr->ai_addr, addr->ai_addrlen) != -1) {
            switch (addr->ai_family) {
            case AF_INET:
                retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr), addressString, INET6_ADDRSTRLEN);
                break;
            case AF_INET6:
                retval = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in6 *>(addr->ai_addr)->sin6_addr), addressString, INET6_ADDRSTRLEN);
                break;
            default:
                // unknown family
                retval = nullptr;
            }
            close(handle);
            break;
        }
    }
    freeaddrinfo(result);
    return retval != nullptr;
}

void NetworkUtils::doAfterCheckNet(const QString &host, const QString &port, std::function<void(bool)> callback)
{
    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, [callback, watcher]() {
        if (callback)
            callback(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run([host, port, this]() { return checkNetConnection(host, port); }));
}

bool NetworkUtils::parseIp(const QString &mpt, QString &ip, QString &port)
{
    static constexpr char kSmbPort[] { "139" };
    static constexpr char kFtpPort[] { "21" };
    static constexpr char kSftpPort[] { "22" };

    auto s(mpt);
    static QRegularExpression gvfsPref { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/)" };
    static QRegularExpression cifsMptPref { "^/media/[\\s\\S]*/smbmounts/" };   // TODO(xust) smb mount point may be changed.
    if (s.contains(gvfsPref)) {
        s.remove(gvfsPref);   // -> ftp:host=1.2.3.4  smb-share:server=1.2.3.4,share=draw
        bool isFtp = s.startsWith("ftp");
        bool isSftp = s.startsWith("sftp");
        bool isSmb = s.startsWith("smb");

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

    } else if (s.contains(cifsMptPref)) {
        s.remove(cifsMptPref);   // shareFolder on host[_idx]
        port = kSmbPort;
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        auto frags = s.split(" on ", QString::SkipEmptyParts);
#else
        auto frags = s.split(" on ", Qt::SkipEmptyParts);
#endif
        if (frags.count() < 2)
            return false;
        ip = frags[1];
        int splitIdx = ip.lastIndexOf("_");
        if (splitIdx > 0)
            ip = ip.mid(0, splitIdx);
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
        qInfo() << "can not connect url = " << url << " host =  " << host << " port = " << port;

    return busy;
}

NetworkUtils::NetworkUtils(QObject *parent)
    : QObject(parent)
{
}
