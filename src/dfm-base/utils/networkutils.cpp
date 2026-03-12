// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkutils.h"

#include <QtConcurrent>
#include <QFutureWatcher>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libmount.h>

using namespace dfmbase;

static constexpr char kSmbPort[] { "445" };
static constexpr char kSmbPortOther[] { "139" };
static constexpr char kFtpPort[] { "21" };
static constexpr char kSftpPort[] { "22" };

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
    conn.connectToHost(host, port.toUShort());
    bool connected = conn.waitForConnected(msecs);
    conn.close();
    // 如果系统设置了代理，那么QTcpSocket会使用代理去连接目标host，代理可能不能访问目标host
    // 在QTcpSocket使用代理不能访问目标host的情况下，将QTcpSocket设置为不使用代理再次连接host，检查能否访问目标host
    if (!connected) {
        // 检查系统代理设置
        QNetworkProxy proxy = QNetworkProxy::applicationProxy();
        if (proxy.type() == QNetworkProxy::NoProxy)
            return connected;

        conn.setProxy(QNetworkProxy::NoProxy);
        conn.connectToHost(host, port.toUShort());
        connected = conn.waitForConnected(msecs);
        conn.close();
    }
    return connected;
}

bool NetworkUtils::checkNetConnection(const QString &host, QStringList ports, int msecs)
{
    static QString lastPort;
    if (!lastPort.isEmpty() && ports.contains(lastPort)) {
        ports.removeOne(lastPort);
        if (checkNetConnection(host, lastPort, msecs))
            return true;
    }

    for (auto const &port : ports) {
        if (checkNetConnection(host, port, msecs)) {
            lastPort = port;
            return true;
        }
    }

    return false;
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
        if (ports.isEmpty()) {
            qCInfo(logDFMBase) << "Network check skipped: no ports specified for host:" << host;
            return true;   // skip check if ports are empty.
        }

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
    QString s(mpt);
    static QRegularExpression gvfsPref { "(^/run/user/\\d+/gvfs/|^/root/\\.gvfs/)" };
    static QRegularExpression cifsMptPref { "^/(?:run/)?media/[\\s\\S]*/smbmounts/" };

    if (s.contains(gvfsPref)) {
        s.remove(gvfsPref);
    } else if (s.contains(cifsMptPref)) {
        s.remove(cifsMptPref);
    } else {
        auto cifsHost = cifsMountHostInfo();
        for (const auto &mountPoint : cifsHost.keys()) {
            if (mpt.startsWith(mountPoint)) {
                auto hostAndPort = cifsHost.value(mountPoint).split(":");
                if (hostAndPort.isEmpty())
                    continue;
                ip = hostAndPort[0];
                port = hostAndPort.count() > 1 ? hostAndPort[1] : kSmbPort;
                return true;
            }
        }
        return false;
    }

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

bool NetworkUtils::parseIp(const QString &mpt, QString &ip, QStringList &ports)
{
    QString port;
    if (parseIp(mpt, ip, port)) {
        ports.append(port);
        if (port == kSmbPort)
            ports.append(kSmbPortOther);
        return true;
    }
    return false;
}

bool NetworkUtils::checkAllCIFSBusy()
{
    const auto &cifsHost = NetworkUtils::cifsMountHostInfo();
    for (const auto &mountPoint : cifsHost.keys()) {
        if (NetworkUtils::instance()->checkFtpOrSmbBusy(QUrl::fromLocalFile(mountPoint))) {
            return true;
        }
    }

    return false;
}

bool NetworkUtils::checkFtpOrSmbBusy(const QUrl &url)
{
    QString host;
    QStringList ports;
    if (!parseIp(url.path(), host, ports))
        return false;

    auto busy = !checkNetConnection(host, ports);
    if (busy)
        qCWarning(logDFMBase) << "Network connection failed for URL:" << url << "host:" << host << "ports:" << ports;

    return busy;
}

QMap<QString, QString> NetworkUtils::cifsMountHostInfo()
{
    static QMutex mutex;
    static QMap<QString, QString> table;
    struct stat statInfo;
    int result = stat("/proc/mounts", &statInfo);

    QMutexLocker locker(&mutex);
    if (0 == result) {
        static quint32 lastModify = 0;
        if (lastModify != statInfo.st_mtime) {
            lastModify = static_cast<quint32>(statInfo.st_mtime);
            table.clear();
        } else {
            return table;
        }

        libmnt_table *tab { mnt_new_table() };
        libmnt_iter *iter { mnt_new_iter(MNT_ITER_BACKWARD) };

        int ret = mnt_table_parse_mtab(tab, nullptr);
        if (ret != 0) {
            mnt_free_table(tab);
            mnt_free_iter(iter);
            qCWarning(logDFMBase) << "Failed to parse mount table, error code:" << ret;
            return table;
        }

        libmnt_fs *fs = nullptr;
        while (mnt_table_next_fs(tab, iter, &fs) == 0) {
            if (!fs)
                continue;
            // net work mount must start with //
            QString srcHostAndPort = mnt_fs_get_source(fs);
            if (!srcHostAndPort.contains(QRegularExpression("^//")))
                continue;

            const QString &mountPath = mnt_fs_get_target(fs);
            srcHostAndPort = srcHostAndPort.replace(QRegularExpression("^//"), "");
            srcHostAndPort = srcHostAndPort.left(srcHostAndPort.indexOf("/"));
            table.insert(mountPath, srcHostAndPort);
        }

        mnt_free_table(tab);
        mnt_free_iter(iter);
    }
    return table;
}

/*!
 * \brief NetworkUtils::resolveLocalSftpMountUrl
 *
 * When \a url is a GVFS SFTP path (e.g.
 * \c /run/user/1000/gvfs/sftp:host=localhost/home/user/file) and the SFTP
 * server is the local machine, strips the GVFS prefix and returns the
 * corresponding real local \c file:// URL.  In all other cases \a url is
 * returned unchanged.
 *
 * The host-is-local check follows three steps:
 *  1. Loopback literals (\c localhost, \c 127.0.0.1, \c ::1).
 *  2. Numeric IP — compared against every address on all local interfaces via
 *     \c QNetworkInterface::allAddresses().
 */
QUrl NetworkUtils::resolveLocalSftpMountUrl(const QUrl &url)
{
    // Extract host from the SFTP GVFS path segment.
    QString host, port;
    if (!instance()->parseIp(url.path(), host, port))
        return url;

    // Determine whether the SFTP server is the local machine.
    const auto isLocalMachine = [](const QString &h) -> bool {
        // Step 1: loopback literals
        if (h.compare(QLatin1String("localhost"), Qt::CaseInsensitive) == 0
            || h == QLatin1String("127.0.0.1")
            || h == QLatin1String("::1"))
            return true;

        // Step 2: numeric IP — match against all local interface addresses
        const QHostAddress candidate(h);
        if (candidate.isNull())
            return false;

        const auto &localAddrs = QNetworkInterface::allAddresses();
        return std::any_of(localAddrs.cbegin(), localAddrs.cend(),
                           [&candidate](const QHostAddress &la) {
                               return la.isEqual(candidate, QHostAddress::TolerantConversion);
                           });
    };

    if (!isLocalMachine(host))
        return url;

    // Strip the GVFS SFTP mount prefix to obtain the real local path.
    // Supported formats:
    //   /run/user/<uid>/gvfs/sftp:host=<h>[,params]/<local-path>
    //   /root/.gvfs/sftp:host=<h>[,params]/<local-path>
    static const QRegularExpression sftpPrefix(R"(^/run/user/\d+/gvfs/sftp:[^/]*|^/root/\.gvfs/sftp:[^/]*)");
    QString localPath = url.toLocalFile();
    localPath.remove(sftpPrefix);
    if (localPath.isEmpty())
        localPath = QStringLiteral("/");

    qCInfo(logDFMBase) << "resolveLocalSftpMountUrl: self-mounted SFTP detected,"
                          " resolved symlink target to local path:"
                       << localPath << "(original:" << url.toLocalFile() << ")";
    return QFile::exists(localPath) ? QUrl::fromLocalFile(localPath) : url;
}

NetworkUtils::NetworkUtils(QObject *parent)
    : QObject(parent)
{
}
