// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAB_P_H
#define TAB_P_H

#include <dfm-base/dfm_base_global.h>
#include "dfmplugin_titlebar_global.h"

#include <QVariant>
#include <QUrl>
#include <QPointF>
#include <QSharedPointer>

class QDrag;

namespace dfmbase {
class AbstractBaseView;
}

namespace dfmplugin_titlebar {

class TabPrivate
{
public:
    TabPrivate();

    QDrag *dragObject = nullptr;
    QVariant tabData;
    QString tabText;
    QString tabAlias;
    QUrl url;
    QPointF originPos;

    QRect closeButtonRect;
    bool closeButtonHovered = false;
    bool showCloseButton = true;

    int tabIndex = -1;
    int width = 0;
    int height = 0;

    bool hovered = false;
    bool pressed = false;
    bool isDragging = false;
    bool dragOutSide = false;
    bool checked = false;
    bool borderLeft = false;
};

}

#endif // TAB_P_H
