/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef TABPRIVATE_H
#define TABPRIVATE_H

#include "dfm-base/dfm_base_global.h"
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

    DFMBASE_NAMESPACE::AbstractBaseView *currentView = nullptr;

    QDrag *dragObject = nullptr;
    QVariant tabData;
    QString tabText;
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
