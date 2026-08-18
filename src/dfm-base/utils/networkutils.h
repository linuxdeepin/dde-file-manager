// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QMutex>

#include <functional>

namespace dfmbase {

class NetworkUtils : public QObject
{
    Q_OBJECT

public:
    static NetworkUtils *instance();

    bool checkNetConnection(const QString &host, const QString &port, int msecs = 1000, const bool useCache = true);
    bool checkNetConnection(const QString &host, QStringList ports, int msecs = 1000);
    void doAfterCheckNet(const QString &host, const QStringList &ports,
                         std::function<void(bool)> callback = nullptr, int msecs = 3000);
    bool parseIp(const QString &mpt, QString &ip, QString &port);
    bool parseIp(const QString &mpt, QString &ip, QStringList &ports);
    bool checkAllCIFSBusy();
    bool checkFtpOrSmbBusy(const QUrl &url);

    // TTL 缓存：避免短期内对同一 host:port 重复 TCP 连接
    struct NetCacheEntry {
        bool busy { false };
        QDateTime timestamp;
    };
    static constexpr int kNetCacheTTLMs { 500 };   // 500 ms

    static QString makeCacheKey(const QString &host, const QString &port);
    NetCacheEntry getFromCache(const QString &host, const QString &port) const;
    void updateCache(const QString &host, const QString &port, bool busy);
    void clearCache();

    // if network mount，get network mount
    static QMap<QString, QString> cifsMountHostInfo();

    /**
     * @brief resolveLocalSftpMountUrl
     *   If @p url is a GVFS SFTP path whose server is the local machine,
     *   strips the GVFS prefix and returns the real local @c file:// URL.
     *   In all other cases @p url is returned unchanged.
     *
     *   This prevents a deadlock that occurs when a symlink created inside a
     *   self-mounted SFTP share points back into the same GVFS mount hierarchy,
     *   causing the SFTP FUSE daemon to issue a re-entrant request to itself.
     * @param url  Source URL (potentially a GVFS SFTP path).
     * @return     Resolved local URL, or @p url unchanged when not applicable.
     */
    static QUrl resolveLocalSftpMountUrl(const QUrl &url);

private:
    QString hexIpToString(const QString& hexIp);
    bool getHostAndPortByReadNet(QString &host, QStringList &ports);
    mutable QMutex cacheMutex;
    mutable QHash<QString, NetCacheEntry> netCache;

protected:
    explicit NetworkUtils(QObject *parent = nullptr);
};

}

#endif   // NETWORKUTILS_H
