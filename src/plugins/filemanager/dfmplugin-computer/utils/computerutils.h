/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef COMPUTERUTILS_H
#define COMPUTERUTILS_H

#include "dfmplugin_computer_global.h"

#include <QString>
#include <QIcon>
#include <QUrl>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerUtils
{
public:
    inline static QString scheme()
    {
        return "computer";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("computer-symbolic");
    }

    inline static QUrl rootUrl()
    {
        QUrl u;
        u.setScheme(scheme());
        u.setPath("/");
        return u;
    }
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERUTILS_H
