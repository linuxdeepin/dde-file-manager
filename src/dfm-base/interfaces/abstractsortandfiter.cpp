// SPDX-FileCopyrightText: 2023 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractsortandfiter.h"

using namespace dfmbase;

AbstractSortAndFiter::AbstractSortAndFiter()
{

}

int AbstractSortAndFiter::lessThan(const FileInfoPointer &left, const FileInfoPointer &right, const bool isMixDirAndFile, const Global::ItemRoles role, const SortScenarios ss)
{
    Q_UNUSED(left)
    Q_UNUSED(right)
    Q_UNUSED(isMixDirAndFile)
    Q_UNUSED(role)
    Q_UNUSED(ss)
    return -1;
}

int AbstractSortAndFiter::checkFiters(const FileInfoPointer &info, const QDir::Filters filter, const QVariant &custum)
{
    Q_UNUSED(info)
    Q_UNUSED(filter)
    Q_UNUSED(custum)
    return -1;
}
