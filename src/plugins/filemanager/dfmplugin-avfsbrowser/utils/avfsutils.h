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
#ifndef AVFSUTILS_H
#define AVFSUTILS_H

#include "dfmplugin_avfsbrowser_global.h"

#include <QString>
#include <QUrl>

DPAVFSBROWSER_BEGIN_NAMESPACE

class AvfsUtils
{
public:
    static inline QString scheme()
    {
        return "avfs";
    }

    static QUrl urlConvert(const QUrl &avfsUrl);
};

DPAVFSBROWSER_END_NAMESPACE

#endif   // AVFSUTILS_H
