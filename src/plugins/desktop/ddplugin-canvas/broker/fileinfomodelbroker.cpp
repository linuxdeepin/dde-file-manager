// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileinfomodelbroker.h"
#include "model/fileinfomodel.h"

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QUrl *)
Q_DECLARE_METATYPE(QModelIndex *)
Q_DECLARE_METATYPE(QSharedPointer<DFMBASE_NAMESPACE::FileInfo> *)
Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace ddplugin_canvas;

#define FileInfoModelPublish(topic, args...) \
    dpfSignalDispatcher->publish(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), ##args)

#define FileInfoModelSlot(topic, args...) \
    dpfSlotChannel->connect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define FileInfoModelDisconnect(topic) \
    dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic))

FileInfoModelBroker::FileInfoModelBroker(FileInfoModel *m, QObject *parent)
    : QObject(parent), model(m)
{
}

FileInfoModelBroker::~FileInfoModelBroker()
{
    FileInfoModelDisconnect(slot_FileInfoModel_RootUrl);
    FileInfoModelDisconnect(slot_FileInfoModel_RootIndex);
    FileInfoModelDisconnect(slot_FileInfoModel_UrlIndex);
    FileInfoModelDisconnect(slot_FileInfoModel_IndexUrl);
    FileInfoModelDisconnect(slot_FileInfoModel_Files);
    FileInfoModelDisconnect(slot_FileInfoModel_FileInfo);
    FileInfoModelDisconnect(slot_FileInfoModel_Refresh);
    FileInfoModelDisconnect(slot_FileInfoModel_ModelState);
    FileInfoModelDisconnect(slot_FileInfoModel_UpdateFile);
}

bool FileInfoModelBroker::init()
{
    // signal
    connect(model, &FileInfoModel::dataReplaced, this, &FileInfoModelBroker::onDataReplaced, Qt::DirectConnection);

    FileInfoModelSlot(slot_FileInfoModel_RootUrl, &FileInfoModelBroker::rootUrl);
    FileInfoModelSlot(slot_FileInfoModel_RootIndex, &FileInfoModelBroker::rootIndex);
    FileInfoModelSlot(slot_FileInfoModel_UrlIndex, &FileInfoModelBroker::urlIndex);
    FileInfoModelSlot(slot_FileInfoModel_IndexUrl, &FileInfoModelBroker::indexUrl);
    FileInfoModelSlot(slot_FileInfoModel_Files, &FileInfoModelBroker::files);
    FileInfoModelSlot(slot_FileInfoModel_FileInfo, &FileInfoModelBroker::fileInfo);
    FileInfoModelSlot(slot_FileInfoModel_Refresh, &FileInfoModelBroker::refresh);
    FileInfoModelSlot(slot_FileInfoModel_ModelState, &FileInfoModelBroker::modelState);
    FileInfoModelSlot(slot_FileInfoModel_UpdateFile, &FileInfoModelBroker::updateFile);

    return true;
}

QUrl FileInfoModelBroker::rootUrl()
{
    return model->rootUrl();
}

QModelIndex FileInfoModelBroker::rootIndex()
{
    return model->rootIndex();
}

QModelIndex FileInfoModelBroker::urlIndex(const QUrl &url)
{
    return model->index(url);
}

FileInfoPointer FileInfoModelBroker::fileInfo(const QModelIndex &index)
{
    return model->fileInfo(index);
}

QUrl FileInfoModelBroker::indexUrl(const QModelIndex &index)
{
    return model->fileUrl(index);
}

QList<QUrl> FileInfoModelBroker::files()
{
    return model->files();
}

void FileInfoModelBroker::refresh(const QModelIndex &parent)
{
    model->refresh(parent);
}

int FileInfoModelBroker::modelState()
{
    return model->modelState();
}

void FileInfoModelBroker::updateFile(const QUrl &url)
{
    return model->updateFile(url);
}

void FileInfoModelBroker::onDataReplaced(const QUrl &oldUrl, const QUrl &newUrl)
{
    FileInfoModelPublish(signal_FileInfoModel_DataReplaced, oldUrl, newUrl);
}
