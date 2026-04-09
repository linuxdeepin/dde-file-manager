// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXCONTROLLERDESCRIPTOR_H
#define INDEXCONTROLLERDESCRIPTOR_H

#include "daemonplugin_core_global.h"

#include <functional>

class QDBusAbstractInterface;

DAEMONPCORE_BEGIN_NAMESPACE

struct IndexControllerDescriptor
{
    QString controllerName;
    QString dbusServiceName;
    QString dbusObjectPath;
    QString configPath;
    QString enableKey;
    std::function<QStringList()> indexedPathsProvider;
    std::function<QDBusAbstractInterface *(QObject *parent)> interfaceFactory;
};

DAEMONPCORE_END_NAMESPACE

#endif   // INDEXCONTROLLERDESCRIPTOR_H
