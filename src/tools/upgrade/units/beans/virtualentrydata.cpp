// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualentrydata.h"

#include <QUrl>

using namespace dfm_upgrade;

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
    : key(other.key), host(other.host), protocol(other.protocol), port(other.port), displayName(other.displayName)
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
    if (key == newKey)
        return;
    key = newKey;
    emit keyChanged();
}

const QString &VirtualEntryData::getProtocol() const
{
    return protocol;
}

void VirtualEntryData::setProtocol(const QString &newProtocol)
{
    if (protocol == newProtocol)
        return;
    protocol = newProtocol;
    emit protocolChanged();
}

const QString &VirtualEntryData::getHost() const
{
    return host;
}

void VirtualEntryData::setHost(const QString &newHost)
{
    if (host == newHost)
        return;
    host = newHost;
    emit hostChanged();
}

int VirtualEntryData::getPort() const
{
    return port;
}

void VirtualEntryData::setPort(int newPort)
{
    if (port == newPort)
        return;
    port = newPort;
    emit portChanged();
}

const QString &VirtualEntryData::getDisplayName() const
{
    return displayName;
}

void VirtualEntryData::setDisplayName(const QString &newDisplayName)
{
    if (displayName == newDisplayName)
        return;
    displayName = newDisplayName;
    emit displayNameChanged();
}
