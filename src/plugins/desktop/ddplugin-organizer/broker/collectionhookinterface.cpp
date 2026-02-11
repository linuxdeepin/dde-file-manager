// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddplugin_organizer_global.h"
#include "collectionhookinterface.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QMimeData>
#include <QPoint>
#include <QPainter>
#include <QStyleOptionViewItem>

Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(QMimeData *)
Q_DECLARE_METATYPE(const QStyleOptionViewItem *)

#define CollectionRunHook(topic, args...) \
            dpfHookSequence->run(QT_STRINGIFY(DDP_ORGANIZER_NAMESPACE), QT_STRINGIFY2(topic), ##args)

using namespace ddplugin_organizer;

bool CollectionHookInterface::dropData(const QString &viewId, const QMimeData *md, const QPoint &viewPoint, void *extData)
{
    return CollectionRunHook(hook_CollectionView_DropData, viewId, md, viewPoint, extData);
}

bool CollectionHookInterface::keyPress(const QString &viewId, int key, int modifiers, void *extData)
{
    return CollectionRunHook(hook_CollectionView_KeyPress, viewId, key, modifiers, extData);
}

bool CollectionHookInterface::startDrag(const QString &viewId, int supportedActions, void *extData)
{
    return CollectionRunHook(hook_CollectionView_StartDrag, viewId, supportedActions, extData);
}

bool CollectionHookInterface::dragMove(const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData)
{
    return CollectionRunHook(hook_CollectionView_DragMove, viewId, mime, viewPos, extData);
}

bool CollectionHookInterface::keyboardSearch(const QString &viewId, const QString &search, void *extData)
{
    return CollectionRunHook(hook_CollectionView_KeyboardSearch, viewId, search, extData);
}

bool CollectionHookInterface::drawFile(const QString &viewId, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData)
{
    return CollectionRunHook(hook_CollectionView_DrawFile, viewId, file, painter, option, extData);
}

bool CollectionHookInterface::shortcutkeyPress(const QString &viewId, int key, int modifiers, void *extData)
{
    return CollectionRunHook(hook_CollectionView_ShortcutKeyPress, viewId, key, modifiers, extData);
}
