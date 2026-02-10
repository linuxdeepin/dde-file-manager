// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CIFSMOUNTHELPER_P_H
#define CIFSMOUNTHELPER_P_H

#include "service_mountcontrol_global.h"

#include <QLibrary>

SERVICEMOUNTCONTROL_BEGIN_NAMESPACE

typedef void *(*FnSmbcNewContext)(void);
typedef int (*FnSmbcFreeContext)(void *context, int shutdownCtx);
typedef const char *(*FnSmbcNegprot)(const char *host,
                                     unsigned short port,
                                     unsigned int timeout_msec,
                                     const char *min_protocol,
                                     const char *max_protocol);
typedef int (*FnSmbcResolveHost)(const char *host,
                                 uint16_t port,
                                 uint32_t timeout_msec,
                                 void *buf,
                                 size_t bufsize);

class SmbcAPI
{
public:
    SmbcAPI();
    ~SmbcAPI();

    bool isInitialized() const;

    FnSmbcNegprot getSmbcNegprot() const;
    FnSmbcResolveHost getSmbcResolveHost() const;

    static QMap<QString, QString> versionMapper();

private:
    void init();

    bool initialized { false };

    QLibrary *libSmbc { nullptr };
    FnSmbcNewContext smbcNewContext { nullptr };
    FnSmbcFreeContext smbcFreeContext { nullptr };
    FnSmbcNegprot smbcNegprot { nullptr };
    FnSmbcResolveHost smbcResolveHost { nullptr };

    void *smbcCtx { nullptr };
};

class CifsMountHelperPrivate
{
    friend class CifsMountHelper;
    SmbcAPI smbcAPI;

public:
    QString probeVersion(const QString &host, ushort port);
    QString parseIP(const QString &host, uint16_t port);
    QString parseIP_old(const QString &host);
};

SERVICEMOUNTCONTROL_END_NAMESPACE

#endif   // CIFSMOUNTHELPER_P_H
