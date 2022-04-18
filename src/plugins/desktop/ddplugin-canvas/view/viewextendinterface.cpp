/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "viewextendinterface.h"

DDP_CANVAS_USE_NAMESPACE

ViewExtendInterface::ViewExtendInterface()
{

}

ViewExtendInterface::~ViewExtendInterface()
{

}

bool ViewExtendInterface::contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewExtendInterface::dropData(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewExtendInterface::mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewExtendInterface::wheel(int viewIndex, const QPoint &angleDelta, void *extData) const
{
    return false;
}

bool ViewExtendInterface::startDrag(int viewIndex, int supportedActions, void *extData) const
{
    return false;
}

bool ViewExtendInterface::dragEnter(int viewIndex, const QMimeData *, void *extData) const
{
    return false;
}

bool ViewExtendInterface::dragMove(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewExtendInterface::dragLeave(int viewIndex, const QMimeData *, void *extData) const
{
    return false;
}

bool ViewExtendInterface::keyboardSearch(int viewIndex, const QString &search, void *extData) const
{
    return false;
}

bool ViewExtendInterface::drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData) const
{
    return false;
}

void ViewExtendInterface::sigIconSizeChanged(int level, void *extData) const
{
    return;
}

void ViewExtendInterface::sigHiddenFlagChanged(bool hidden, void *extData) const
{
    return;
}

bool ViewExtendInterface::keyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return false;
}

bool ViewExtendInterface::mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewExtendInterface::mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}
