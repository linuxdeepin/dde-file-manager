// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QThread>
#include <QString>
#include <QStringList>

#include <algorithm>

namespace dock {

// Guards GUI-thread file access against unreachable network mounts.
//
// A pinned folder may live inside a GVFS (smb/ftp/sftp) or kernel CIFS mount.
// When the remote host becomes unreachable, every stat/open/list operation on
// those paths blocks the calling thread until the network timeout expires
// (typically tens of seconds), which would freeze the dock UI. This helper
// mirrors the file manager's dfm-base NetworkUtils pre-check: before touching
// such a path, resolve its host:port from the mount name and probe it with a
// short TCP connect. Callers skip the operation while the probe reports the
// host unreachable.
//
// The class is header-only so the standalone test build (which does not link
// dfm-base) can exercise the parsing logic directly. All methods may be
// called from the GUI thread only: the probe blocks its caller for at most
// ~1s per port and is amortised by a TTL cache (500ms for reachable hosts,
// 5s for unreachable ones so repeated clicks do not re-block every time).
class NetworkGuard
{
public:
    static NetworkGuard &instance()
    {
        static NetworkGuard guard;
        return guard;
    }

    NetworkGuard(const NetworkGuard &) = delete;
    NetworkGuard &operator=(const NetworkGuard &) = delete;

    // True when @p path sits on a network mount whose host is currently
    // unreachable. Always false for local paths (cheap regular-expression
    // check, no probing).
    bool isBusy(const QString &path)
    {
        Q_ASSERT(!QCoreApplication::instance()
                 || QThread::currentThread() == QCoreApplication::instance()->thread());

        QString host;
        QStringList ports;
        if (!parseHostPort(path, &host, &ports))
            return false;

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        {
            QMutexLocker locker(&m_cacheMutex);
            bool allCached = true;
            bool allBusy = true;
            for (const QString &port : ports) {
                const auto it = m_cache.constFind(cacheKey(host, port));
                if (it == m_cache.constEnd()) {
                    allCached = false;
                    break;
                }
                const qint64 ttl = it->busy ? kBusyCacheTTLMs : kReachableCacheTTLMs;
                if (now - it->timestampMs >= ttl) {
                    allCached = false;
                    break;
                }
                if (!it->busy)
                    allBusy = false;
            }
            if (allCached)
                return allBusy;
        }

        const bool reachable = probeTcp(host, ports);
        {
            QMutexLocker locker(&m_cacheMutex);
            const qint64 stamp = QDateTime::currentMSecsSinceEpoch();
            for (const QString &port : ports)
                m_cache.insert(cacheKey(host, port), { !reachable, stamp });
        }
        return !reachable;
    }

    // True when @p path sits on a known network mount, without probing.
    bool isNetworkPath(const QString &path)
    {
        QString host;
        QStringList ports;
        return parseHostPort(path, &host, &ports);
    }

    // Resolve the remote host and candidate ports a path's mount connects to.
    // Exposed for tests. Returns false for local paths.
    static bool parseHostPort(const QString &path, QString *host, QStringList *ports)
    {
        static const QRegularExpression gvfsPrefix { "(^/run/user/\\d+/gvfs/|^/root/\\.gvfs/)" };
        static const QRegularExpression cifsMountPrefix { "^/(?:run/)?media/[^/]*/smbmounts/" };

        QString s = path;
        if (s.contains(gvfsPrefix)) {
            s.remove(gvfsPrefix);
        } else if (s.contains(cifsMountPrefix)) {
            s.remove(cifsMountPrefix);
        } else {
            const auto cifsHosts = cifsMountHosts();
            for (auto it = cifsHosts.constBegin(); it != cifsHosts.constEnd(); ++it) {
                if (!path.startsWith(it.key()))
                    continue;
                const QStringList hostAndPort = it.value().split(QLatin1Char(':'));
                if (hostAndPort.isEmpty())
                    continue;
                *host = hostAndPort.first();
                *ports = hostAndPort.size() > 1 ? QStringList { hostAndPort.at(1) }
                                                : QStringList { kSmbPort };
                return true;
            }
            return false;
        }

        // s = "ftp:host=1.2.3.4" or "smb-share:server=1.2.3.4,share=draw"
        const bool isFtp = s.startsWith(QLatin1String("ftp"));
        const bool isSftp = s.startsWith(QLatin1String("sftp"));
        const bool isSmb = s.startsWith(QLatin1String("smb"));
        if (!isFtp && !isSftp && !isSmb)
            return false;

        // "smb-share:port=321,server=1.2.3.4,share=draw" — port is optional
        // and may appear before or after the server key.
        static const QRegularExpression hostAndPortRegx(
                R"(([:,]port=(?<port0>\d*))?[,:](server|host)=(?<host>[^/:,]+)(,port=(?<port1>\d*))?)");
        const auto match = hostAndPortRegx.match(s);
        if (!match.hasMatch())
            return false;

        QString port = match.captured("port0");
        if (port.isEmpty())
            port = match.captured("port1");
        if (port.isEmpty())
            port = isFtp ? kFtpPort : isSftp ? kSftpPort : kSmbPort;
        *host = match.captured("host");
        *ports = QStringList { port };
        if (port == kSmbPort)
            *ports << kSmbPortOther;   // legacy SMB port as fallback
        return true;
    }

private:
    NetworkGuard() = default;

    static QString cacheKey(const QString &host, const QString &port)
    {
        return host + QLatin1Char(':') + port;
    }

    // Try the candidate ports in order; one successful connect means the
    // host is reachable. Mirrors NetworkUtils::checkNetConnection, including
    // the retry without proxy when an application proxy blocks the direct
    // connection.
    bool probeTcp(const QString &host, const QStringList &ports)
    {
        QMutexLocker locker(&m_probeMutex);
        static QString lastGoodPort;
        QStringList ordered = ports;
        if (!lastGoodPort.isEmpty() && ordered.contains(lastGoodPort)) {
            ordered.removeOne(lastGoodPort);
            ordered.prepend(lastGoodPort);
        }
        for (const QString &port : std::as_const(ordered)) {
            QTcpSocket conn;
            conn.connectToHost(host, port.toUShort());
            bool connected = conn.waitForConnected(kProbeTimeoutMs);
            conn.close();
            if (!connected) {
                const QNetworkProxy proxy = QNetworkProxy::applicationProxy();
                if (proxy.type() != QNetworkProxy::NoProxy) {
                    conn.setProxy(QNetworkProxy::NoProxy);
                    conn.connectToHost(host, port.toUShort());
                    connected = conn.waitForConnected(kProbeTimeoutMs);
                    conn.close();
                }
            }
            if (connected) {
                lastGoodPort = port;
                return true;
            }
        }
        return false;
    }

    // Kernel CIFS mounts ("//host/share /target cifs ...") from /proc/mounts:
    // mount point -> host[:port]. Cached on the file's mtime like the file
    // manager does with libmount; plain-text parsing avoids that dependency.
    static QHash<QString, QString> cifsMountHosts()
    {
        static QMutex mutex;
        static QHash<QString, QString> table;
        static qint64 lastStamp = -1;

        const qint64 stamp = QFileInfo(QStringLiteral("/proc/mounts")).lastModified().toMSecsSinceEpoch();
        QMutexLocker locker(&mutex);
        if (stamp != lastStamp) {
            lastStamp = stamp;
            table.clear();
            QFile mounts(QStringLiteral("/proc/mounts"));
            if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
                while (!mounts.atEnd()) {
                    // "src target fstype opts ..." — spaces inside a field are
                    // octal-escaped (\040), so a plain split is safe.
                    const QList<QByteArray> fields = mounts.readLine().split(' ');
                    if (fields.size() < 3)
                        continue;
                    const QString source = unescapeMountField(fields.at(0));
                    if (!source.startsWith(QLatin1String("//")))
                        continue;
                    QString hostPort = source.mid(2);
                    const int slash = hostPort.indexOf(QLatin1Char('/'));
                    if (slash > 0)
                        hostPort.truncate(slash);
                    if (!hostPort.isEmpty())
                        table.insert(unescapeMountField(fields.at(1)), hostPort);
                }
            }
        }
        return table;
    }

    // Decode the octal escapes (/proc/mounts uses \040 space, \011 tab,
    // \012 newline, \134 backslash) applied to fields with special chars.
    static QString unescapeMountField(const QByteArray &field)
    {
        QString out = QString::fromLocal8Bit(field);
        for (int i = 0; i < out.size() - 3;) {
            if (out.at(i) != QLatin1Char('\\')) {
                ++i;
                continue;
            }
            bool ok = false;
            const int code = out.mid(i + 1, 3).toInt(&ok, 8);
            if (!ok) {
                ++i;
                continue;
            }
            out.replace(i, 4, QChar(code));
        }
        return out;
    }

    struct CacheEntry {
        bool busy = false;
        qint64 timestampMs = 0;
    };

    static constexpr int kProbeTimeoutMs { 1000 };          // per port, like the file manager
    static constexpr qint64 kReachableCacheTTLMs { 500 };   // same TTL as the file manager
    static constexpr qint64 kBusyCacheTTLMs { 5000 };       // stickier: avoid re-blocking on retry
    static constexpr char kSmbPort[] { "445" };
    static constexpr char kSmbPortOther[] { "139" };
    static constexpr char kFtpPort[] { "21" };
    static constexpr char kSftpPort[] { "22" };

    QMutex m_cacheMutex;
    QHash<QString, CacheEntry> m_cache;
    QMutex m_probeMutex;
};

}   // namespace dock
