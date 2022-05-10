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
#include "fileinfomodelshell.h"
#include "interface/canvasinterface.h"

#include "dfm-framework/dpf.h"

#include <QModelIndex>

DDP_ORGANIZER_USE_NAMESPACE

#define FileInfoModelSubscribe(topic, func) \
        dpfSignalDispatcher->subscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

#define FileInfoModelUnsubscribe(topic, func) \
        dpfSignalDispatcher->unsubscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

#define FileInfoModelPush(topic) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define FileInfoModelPush2(topic, args...) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

FileInfoModelShell::FileInfoModelShell(QObject *parent)
    : QObject(parent)
{
}

FileInfoModelShell::~FileInfoModelShell()
{
    FileInfoModelUnsubscribe(signal_FileInfoModel_DataReplaced, &FileInfoModelShell::dataReplaced);
}

bool FileInfoModelShell::initialize()
{
    FileInfoModelSubscribe(signal_FileInfoModel_DataReplaced, &FileInfoModelShell::dataReplaced);
    return true;
}

QAbstractItemModel *FileInfoModelShell::sourceModel() const
{
    if (!model) {
        QVariant ret = FileInfoModelPush(slot_CanvasManager_FileInfoModel);
        if (auto ptr = ret.value<QAbstractItemModel *>())
            model = ptr;
        else
            qCritical() << "get fileInfoModel is nullptr";
    }

    return model;
}

QModelIndex FileInfoModelShell::index(const QUrl &url, int column) const
{
    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_UrlIndex, url, column);
    return ret.toModelIndex();
}

DFMLocalFileInfoPointer FileInfoModelShell::fileInfo(const QModelIndex &index) const
{
    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_FileInfo, index);
    return ret.value<DFMLocalFileInfoPointer>();
}

QUrl FileInfoModelShell::fileUrl(const QModelIndex &index) const
{
    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_IndexUrl, index);
    return ret.toUrl();
}

QList<QUrl> FileInfoModelShell::files() const
{
    QVariant ret = FileInfoModelPush(slot_FileInfoModel_Files);
    return ret.value<QList<QUrl>>();
}
