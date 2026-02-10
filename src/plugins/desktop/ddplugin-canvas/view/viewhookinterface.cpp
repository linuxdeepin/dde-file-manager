// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewhookinterface.h"

using namespace ddplugin_canvas;

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

bool ViewHookInterface::shortcutkeyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return false;
}

bool ViewHookInterface::shortcutAction(int viewIndex, int keySequence, void *extData) const
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
