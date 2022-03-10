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
#ifndef SHAREUTILS_H
#define SHAREUTILS_H

#include "dfmplugin_myshares_global.h"

#include <QString>
#include <QIcon>
#include <QUrl>

DPMYSHARES_BEGIN_NAMESPACE

class ShareUtils
{
public:
    static QString scheme();
    static QIcon icon();
    static QString displayName();
    static QUrl rootUrl();
    static QUrl makeShareUrl(const QString &path);
    static QUrl convertToLocalUrl(const QUrl &shareUrl);
};

DPMYSHARES_END_NAMESPACE

#endif   // SHAREUTILS_H
