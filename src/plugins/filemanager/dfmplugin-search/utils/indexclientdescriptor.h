// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXCLIENTDESCRIPTOR_H
#define INDEXCLIENTDESCRIPTOR_H

#include "dfmplugin_search_global.h"

#include <functional>

class QDBusAbstractInterface;

DPSEARCH_BEGIN_NAMESPACE

struct IndexClientDescriptor
{
    QString clientName;
    QString dbusServiceName;
    QString dbusObjectPath;
    std::function<QDBusAbstractInterface *(QObject *parent)> interfaceFactory;
};

DPSEARCH_END_NAMESPACE

#endif   // INDEXCLIENTDESCRIPTOR_H
