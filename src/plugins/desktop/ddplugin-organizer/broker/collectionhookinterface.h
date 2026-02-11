// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONHOOKINTERFACE_H
#define COLLECTIONHOOKINTERFACE_H

#include <QObject>

class QMimeData;
class QPainter;
class QStyleOptionViewItem;
namespace ddplugin_organizer {

class CollectionHookInterface
{
public:
   static bool dropData(const QString &viewId, const QMimeData *, const QPoint &viewPos, void *extData = nullptr);
   static bool keyPress(const QString &viewId, int key, int modifiers, void *extData = nullptr);
   static bool startDrag(const QString &viewId, int supportedActions, void *extData = nullptr);
   static bool dragMove(const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData = nullptr);
   static bool keyboardSearch(const QString &viewId, const QString &search, void *extData = nullptr);
   static bool drawFile(const QString &viewId, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData = nullptr);
   static bool shortcutkeyPress(const QString &viewId, int key, int modifiers, void *extData = nullptr);
};
}

#endif // COLLECTIONHOOKINTERFACE_H
