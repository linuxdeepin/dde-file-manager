/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHEVENTCALLER_H
#define TRASHEVENTCALLER_H

#include "dfmplugin_trash_global.h"

#include <QObject>

DPTRASH_BEGIN_NAMESPACE

class TrashEventCaller
{
    TrashEventCaller() = delete;

public:
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls);
    static void sendEmptyTrash(const quint64 windowId, const QList<QUrl> &urls);
    static void sendTrashPropertyDialog(const QUrl &url);
    static void sendShowEmptyTrash(quint64 winId, bool visible);
};

DPTRASH_END_NAMESPACE

#endif   // TRASHEVENTCALLER_H
