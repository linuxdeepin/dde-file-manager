// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gridcore.h"

GridCore::GridCore()
{

}

QStringList GridCore::reloacle(int screenNum, GIndex targetIndex, int emptyBefore, int emptyAfter)
{
    QStringList dodgeItems;
    auto end = findEmptyBackward(screenNum, targetIndex, emptyAfter);
    dodgeItems << reloacleBackward(screenNum, targetIndex, end);

    auto start = findEmptyForward(screenNum, targetIndex - 1, emptyBefore);
    dodgeItems << reloacleForward(screenNum, start, targetIndex - 1);
    return dodgeItems;
}
