// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfomodelshell.h"
#include "interface/canvasinterface.h"

#include <dfm-framework/dpf.h>

#include <QModelIndex>

using namespace ddplugin_organizer;

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
            fmCritical() << "get fileInfoModel is nullptr";
    }

    return model;
}

QUrl FileInfoModelShell::rootUrl() const
{
    return FileInfoModelPush(slot_FileInfoModel_RootUrl).toUrl();
}

QModelIndex FileInfoModelShell::rootIndex() const
{
    return FileInfoModelPush(slot_FileInfoModel_RootIndex).toModelIndex();
}

QModelIndex FileInfoModelShell::index(const QUrl &url, int column) const
{
    Q_UNUSED(column)

    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_UrlIndex, url);
    return ret.toModelIndex();
}

FileInfoPointer FileInfoModelShell::fileInfo(const QModelIndex &index) const
{
    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_FileInfo, index);
    return ret.value<FileInfoPointer>();
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

void FileInfoModelShell::refresh(const QModelIndex &parent)
{
    FileInfoModelPush2(slot_FileInfoModel_Refresh, parent);
}

int FileInfoModelShell::modelState()
{
    QVariant ret = FileInfoModelPush2(slot_FileInfoModel_ModelState);
    return ret.toInt();
}
