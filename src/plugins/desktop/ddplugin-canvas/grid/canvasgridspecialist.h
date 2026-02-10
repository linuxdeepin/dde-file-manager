// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRIDSPECIALIST_H
#define CANVASGRIDSPECIALIST_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/base/urlroute.h>

#include <QUrl>
#include <QPoint>

namespace ddplugin_canvas {

class CanvasGridSpecialist
{
public:
    //static QHash<QString, QPoint> covertDesktopUrlToFiles(const QHash<QString, QPoint> &);
    static inline bool isValid(const QPoint &pos, const QSize &size)
    {
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

}

#endif   // CANVASGRIDSPECIALIST_H
