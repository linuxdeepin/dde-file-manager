// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONDEFINE_H
#define COMMONDEFINE_H

#include <QPixmap>

namespace dfm_wallpapersetting {

struct ItemNode {
    QString item;
    QPixmap pixmap;
    QColor color;
    bool selectable;
    bool deletable;
};

typedef QSharedPointer<ItemNode> ItemNodePtr;

#define LISTVIEW_ICON_WIDTH 96
#define LISTVIEW_ICON_HEIGHT 54
#define LISTVIEW_ICON_MARGIN 5
#define LISTVIEW_ICON_ROUND 4
#define LISTVIEW_BORDER_ROUND 7
#define LISTVIEW_BORDER_WIDTH 2

#define PREVIEW_ICON_WIDTH 240
#define PREVIEW_ICON_HEIGHT 135
#define PREVIEW_ICON_MARGIN 4
#define PREVIEW_ICON_ROUND 8
#define PREVIEW_BORDER_ROUND 12

}

#endif // COMMONDEFINE_H
