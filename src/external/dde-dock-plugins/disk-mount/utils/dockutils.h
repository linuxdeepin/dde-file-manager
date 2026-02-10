// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKUTILS_H
#define DOCKUTILS_H

#include <QString>
#include <QVariantMap>
#include <QUrl>

namespace common_utils {
bool isIntegratedByFilemanager();
}

namespace size_format {
QString formatDiskSize(const quint64 num);
QString sizeString(const QString &str);
}

namespace smb_utils {
bool parseSmbInfo(const QString &id, QString *host, QString *share, int *port = nullptr);
}

namespace device_utils {
QString blockDeviceName(const QVariantMap &data);
QString protocolDeviceName(const QVariantMap &data);
QString protocolDeviceAlias(const QString &scheme, const QString &host);

QString blockDeviceIcon(const QVariantMap &data);
QString protocolDeviceIcon(const QVariantMap &data);

QUrl blockDeviceTarget(const QVariantMap &data);
QUrl protocolDeviceTarget(const QVariantMap &data);

bool isDlnfsMount(const QString &mpt);
QString queryDevice(const QString &mpt);
}

#endif   // DOCKUTILS_H
