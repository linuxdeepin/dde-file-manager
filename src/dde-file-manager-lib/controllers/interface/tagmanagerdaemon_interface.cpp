// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../../dde-file-manager-lib/controllers/interface/tagmanagerdaemon_interface.h"

/*
 * Implementation of interface class TagManagerDaemonInterface
 */

TagManagerDaemonInterface::TagManagerDaemonInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

TagManagerDaemonInterface::~TagManagerDaemonInterface()
{
}

