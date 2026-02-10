// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualentrydata.h"

#include <QUrl>

DPSMBBROWSER_USE_NAMESPACE

VirtualEntryData::VirtualEntryData(QObject *parent)
    : QObject(parent)
{
}

VirtualEntryData::VirtualEntryData(const QString &standardSmbPath)
{
    QUrl u(standardSmbPath);

    key = standardSmbPath;
    protocol = u.scheme();
    host = u.host();
    port = u.port();
    if (u.path().isEmpty())
        displayName = host;
}

VirtualEntryData::VirtualEntryData(const VirtualEntryData &other)
    : key(other.key), protocol(other.protocol), host(other.host), port(other.port), displayName(other.displayName)
{
}

VirtualEntryData &VirtualEntryData::operator=(const VirtualEntryData &other)
{
    key = other.key;
    host = other.host;
    protocol = other.protocol;
    port = other.port;
    displayName = other.displayName;
    return *this;
}

const QString &VirtualEntryData::getKey() const
{
    return key;
}

void VirtualEntryData::setKey(const QString &newKey)
{
    if (key == newKey) {
        fmDebug() << "Key value unchanged, skipping update:" << newKey;
        return;
    }
    key = newKey;
    emit keyChanged();
}

const QString &VirtualEntryData::getProtocol() const
{
    return protocol;
}

void VirtualEntryData::setProtocol(const QString &newProtocol)
{
    if (protocol == newProtocol) {
        fmDebug() << "Protocol value unchanged, skipping update:" << newProtocol;
        return;
    }
    protocol = newProtocol;
    emit protocolChanged();
}

const QString &VirtualEntryData::getHost() const
{
    return host;
}

void VirtualEntryData::setHost(const QString &newHost)
{
    if (host == newHost) {
        fmDebug() << "Host value unchanged, skipping update:" << newHost;
        return;
    }
    host = newHost;
    emit hostChanged();
}

int VirtualEntryData::getPort() const
{
    return port;
}

void VirtualEntryData::setPort(int newPort)
{
    if (port == newPort) {
        fmDebug() << "Port value unchanged, skipping update:" << newPort;
        return;
    }
    port = newPort;
    emit portChanged();
}

const QString &VirtualEntryData::getDisplayName() const
{
    return displayName;
}

void VirtualEntryData::setDisplayName(const QString &newDisplayName)
{
    if (displayName == newDisplayName) {
        fmDebug() << "DisplayName value unchanged, skipping update:" << newDisplayName;
        return;
    }
    displayName = newDisplayName;
    emit displayNameChanged();
}

const QString &VirtualEntryData::getTargetPath() const
{
    return targetPath;
}

void VirtualEntryData::setTargetPath(const QString &targetPath)
{
    this->targetPath = targetPath;
}
