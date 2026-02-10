// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "licenceInterface.h"
/*
 * Implementation of interface class ComDeepinLicenseInterface
 */

ComDeepinLicenseInterface::ComDeepinLicenseInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

ComDeepinLicenseInterface::~ComDeepinLicenseInterface()
{
}
