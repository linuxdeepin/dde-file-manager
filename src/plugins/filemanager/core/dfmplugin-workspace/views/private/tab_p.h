// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABPRIVATE_H
#define TABPRIVATE_H

#include <dfm-base/dfm_base_global.h>
#include "dfmplugin_workspace_global.h"

#include <QVariant>
#include <QUrl>
#include <QPointF>
#include <QSharedPointer>

class QDrag;

namespace dfmbase {
class AbstractBaseView;
}

namespace dfmplugin_workspace {

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

#endif   // TABPRIVATE_H
