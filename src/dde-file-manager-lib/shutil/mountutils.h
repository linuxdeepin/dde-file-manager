/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef MOUNTUTILS_H
#define MOUNTUTILS_H

#include <QString>
#include <QVariantMap>

class DBlockDevice;
class MountUtils
{
public:
    static bool isSecurityEnhanceOpen();
    static QString getSeLinuxMountParam(bool removable = true);
    static QVariantMap getSeLinuxMountParams(bool removable = true);
    static QString mountBlkWithParams(DBlockDevice *dev);
};

#endif // MOUNTUTILS_H
