/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CANVASGRIDSPECIALIST_H
#define CANVASGRIDSPECIALIST_H

#include "dfm_desktop_service_global.h"

#include "base/urlroute.h"

#include <QUrl>
#include <QPoint>

DSB_D_BEGIN_NAMESPACE

class CanvasGridSpecialist
{
public:
    //static QHash<QString, QPoint> covertDesktopUrlToFiles(const QHash<QString, QPoint> &);
    static inline bool isValid(const QPoint &pos, const QSize &size) {
        return size.width() > pos.x() && size.height() > pos.y()
                && pos.x() >= 0 && pos.y() >= 0;
    }
    static QString profileKey(int index);
    static int profileIndex(QString);
    static QStringList sortItemInGrid(const QHash<QString, QPoint> &);
public:
    static const char *const profilePrefix;
    static const char *const singleIndex;
};

DSB_D_END_NAMESPACE

#endif // CANVASGRIDSPECIALIST_H
