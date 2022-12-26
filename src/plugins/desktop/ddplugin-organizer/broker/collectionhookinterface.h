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
