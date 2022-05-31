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
#include "viewhookinterface.h"

DDP_CANVAS_USE_NAMESPACE

ViewHookInterface::ViewHookInterface()
{

}

ViewHookInterface::~ViewHookInterface()
{

}

bool ViewHookInterface::contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewHookInterface::dropData(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewHookInterface::mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewHookInterface::wheel(int viewIndex, const QPoint &angleDelta, void *extData) const
{
    return false;
}

bool ViewHookInterface::startDrag(int viewIndex, int supportedActions, void *extData) const
{
    return false;
}

bool ViewHookInterface::dragEnter(int viewIndex, const QMimeData *, void *extData) const
{
    return false;
}

bool ViewHookInterface::dragMove(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewHookInterface::dragLeave(int viewIndex, const QMimeData *, void *extData) const
{
    return false;
}

bool ViewHookInterface::keyboardSearch(int viewIndex, const QString &search, void *extData) const
{
    return false;
}

bool ViewHookInterface::drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData) const
{
    return false;
}

bool ViewHookInterface::keyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return false;
}

bool ViewHookInterface::mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}

bool ViewHookInterface::mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return false;
}
