// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmenuscene_p.h"

#include <QWidget>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

TagMenuScenePrivate::TagMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

QWidget *TagMenuScenePrivate::getSurface(QWidget *w, QRect *rect)
{
    QWidget *surface = nullptr;
    if (w) {
        QWidget *p = w;
        while (p = p->parentWidget()) {
            if (p->property("WidgetName").toString() == QString("organizersurface")) {
                surface = p;
                if (rect)
                    *rect = p->rect();
                break;
            }
        }
    }

    return surface;
}
