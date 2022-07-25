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

#include "canvasmodelhook.h"
#include "model/canvasproxymodel.h"

#include <dfm-framework/dpf.h>

#include <QVariant>
#include <QMimeData>

// register type
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(QStringList *)
Q_DECLARE_METATYPE(QMimeData *)

using namespace ddplugin_canvas;

#define CanvasModelRunHook(topic, args...) \
            dpfHookSequence->run(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), ##args)

CanvasModelHook::CanvasModelHook(QObject *parent)
    : QObject(parent)
    , ModelHookInterface()
{

}

bool CanvasModelHook::modelData(const QUrl &url, int role, QVariant *out, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_Data, url, role, out, extData);
}

bool CanvasModelHook::dataInserted(const QUrl &url, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DataInserted, url, extData);
}

bool CanvasModelHook::dataRemoved(const QUrl &url, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DataRemoved, url, extData);
}

bool CanvasModelHook::dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DataRenamed, oldUrl, newUrl, extData);
}

bool CanvasModelHook::dataRested(QList<QUrl> *urls, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DataRested, urls, extData);
}

bool CanvasModelHook::dataChanged(const QUrl &url, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DataChanged, url, extData);
}

bool CanvasModelHook::dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_DropMimeData, data, dir, action, extData);
}

bool CanvasModelHook::mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_MimeData, urls, out, extData);
}

bool CanvasModelHook::mimeTypes(QStringList *types, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_MimeTypes, types, extData);
}

bool CanvasModelHook::sortData(int role, int order, QList<QUrl> *files, void *extData) const
{
    return CanvasModelRunHook(hook_CanvasModel_SortData, role, order, files, extData);
}
