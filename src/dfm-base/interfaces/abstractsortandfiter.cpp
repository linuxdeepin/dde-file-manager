// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractsortandfiter.h"

using namespace dfmbase;

AbstractSortAndFiter::AbstractSortAndFiter()
{

}

int AbstractSortAndFiter::lessThan(const AbstractFileInfoPointer &left, const AbstractFileInfoPointer &right, const bool isMixDirAndFile, const Global::ItemRoles role, const Qt::SortOrder sortOder, const SortScenarios ss)
{
    Q_UNUSED(left)
    Q_UNUSED(right)
    Q_UNUSED(isMixDirAndFile)
    Q_UNUSED(sortOder)
    Q_UNUSED(role)
    Q_UNUSED(ss)
    return true;
}

int AbstractSortAndFiter::checkFiters(const AbstractFileInfoPointer &info, const QDir::Filters filter)
{
    Q_UNUSED(info)
    Q_UNUSED(filter)
    return -1;
}
