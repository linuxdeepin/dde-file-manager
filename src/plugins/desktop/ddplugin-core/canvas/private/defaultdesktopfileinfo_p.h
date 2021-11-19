/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DEFAULTDESKTOPFILEINFO_P_H
#define DEFAULTDESKTOPFILEINFO_P_H

#include "defaultdesktopfileinfo.h"

DSB_D_BEGIN_NAMESPACE
class DefaultDesktopFileInfoPrivate
{
public:
    DefaultDesktopFileInfoPrivate() {}
    ~DefaultDesktopFileInfoPrivate() {}

public:
    mutable QMimeType mimeType;
    mutable QMimeDatabase::MatchMode mimeTypeMode;
    mutable QIcon icon;
    mutable bool iconFromTheme { false };
    // 小于0时表示此值未初始化，0表示不支持，1表示支持
    mutable qint8 hasThumbnail { -1 };
    QString iconName;
};

DSB_D_END_NAMESPACE
#endif   // DEFAULTDESKTOPFILEINFO_P_H
