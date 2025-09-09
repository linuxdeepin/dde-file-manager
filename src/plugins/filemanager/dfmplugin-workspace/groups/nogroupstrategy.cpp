// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nogroupstrategy.h"

#include <dfm-base/dfm_global_defines.h>

#include <QCoreApplication>

DPWORKSPACE_BEGIN_NAMESPACE

using namespace DFMBASE_NAMESPACE;

NoGroupStrategy::NoGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
}

NoGroupStrategy::~NoGroupStrategy()
{
}

QString NoGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    Q_UNUSED(info)
    // All files belong to the same virtual group in "no grouping" mode
    return QString::fromLatin1(kNoGroupKey);
}

QString NoGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    Q_UNUSED(groupKey)
    // No display name for the no-group mode - groups are not shown
    return QString();
}

QStringList NoGroupStrategy::getGroupOrder(Qt::SortOrder order) const
{
    Q_UNUSED(order)
    // Only one virtual group exists
    return QStringList() << QString::fromLatin1(kNoGroupKey);
}

int NoGroupStrategy::getGroupDisplayOrder(const QString &groupKey, Qt::SortOrder order) const
{
    Q_UNUSED(groupKey)
    Q_UNUSED(order)
    // Only one group, so it's always at position 0
    return 0;
}

bool NoGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    Q_UNUSED(infos)
    // In no-grouping mode, group headers should not be displayed
    return false;
}

QString NoGroupStrategy::getStrategyName() const
{
    // Return localized strategy name
    return "NoGroupStrategy";
}

Global::ItemRoles NoGroupStrategy::getCorrespondingRole() const
{
    // No specific role corresponds to "no grouping"
    return Global::ItemRoles::kItemUnknowRole;
}

DPWORKSPACE_END_NAMESPACE
