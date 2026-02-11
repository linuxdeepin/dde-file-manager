// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmodelbroker.h"
#include "model/canvasproxymodel.h"

#include <dfm-framework/dpf.h>

using namespace ddplugin_canvas;

#define CanvasModelSlot(topic, args...) \
            dpfSlotChannel->connect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasModelDisconnect(topic) \
            dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic))

CanvasModelBroker::CanvasModelBroker(CanvasProxyModel *m, QObject *parent)
    : QObject(parent)
    , model(m)
{
}

CanvasModelBroker::~CanvasModelBroker()
{
    CanvasModelDisconnect(slot_CanvasModel_RootUrl);
    CanvasModelDisconnect(slot_CanvasModel_UrlIndex);
    CanvasModelDisconnect(slot_CanvasModel_Index);
    CanvasModelDisconnect(slot_CanvasModel_FileUrl);
    CanvasModelDisconnect(slot_CanvasModel_Files);
    CanvasModelDisconnect(slot_CanvasModel_ShowHiddenFiles);
    CanvasModelDisconnect(slot_CanvasModel_SetShowHiddenFiles);
    CanvasModelDisconnect(slot_CanvasModel_SortOrder);
    CanvasModelDisconnect(slot_CanvasModel_SetSortOrder);
    CanvasModelDisconnect(slot_CanvasModel_SortRole);
    CanvasModelDisconnect(slot_CanvasModel_SetSortRole);
    CanvasModelDisconnect(slot_CanvasModel_RowCount);
    CanvasModelDisconnect(slot_CanvasModel_Data);
    CanvasModelDisconnect(slot_CanvasModel_Sort);
    CanvasModelDisconnect(slot_CanvasModel_Refresh);
    CanvasModelDisconnect(slot_CanvasModel_Fetch);
    CanvasModelDisconnect(slot_CanvasModel_Take);
}

bool CanvasModelBroker::init()
{
    CanvasModelSlot(slot_CanvasModel_RootUrl, &CanvasModelBroker::rootUrl);
    CanvasModelSlot(slot_CanvasModel_UrlIndex, &CanvasModelBroker::urlIndex);
    CanvasModelSlot(slot_CanvasModel_Index, &CanvasModelBroker::index);
    CanvasModelSlot(slot_CanvasModel_FileUrl, &CanvasModelBroker::fileUrl);
    CanvasModelSlot(slot_CanvasModel_Files, &CanvasModelBroker::files);
    CanvasModelSlot(slot_CanvasModel_ShowHiddenFiles, &CanvasModelBroker::showHiddenFiles);
    CanvasModelSlot(slot_CanvasModel_SetShowHiddenFiles, &CanvasModelBroker::setShowHiddenFiles);
    CanvasModelSlot(slot_CanvasModel_SortOrder, &CanvasModelBroker::sortOrder);
    CanvasModelSlot(slot_CanvasModel_SetSortOrder, &CanvasModelBroker::setSortOrder);
    CanvasModelSlot(slot_CanvasModel_SortRole, &CanvasModelBroker::sortRole);
    CanvasModelSlot(slot_CanvasModel_SetSortRole, &CanvasModelBroker::setSortRole);
    CanvasModelSlot(slot_CanvasModel_RowCount, &CanvasModelBroker::rowCount);
    CanvasModelSlot(slot_CanvasModel_Data, &CanvasModelBroker::data);
    CanvasModelSlot(slot_CanvasModel_Sort, &CanvasModelBroker::sort);
    CanvasModelSlot(slot_CanvasModel_Refresh, &CanvasModelBroker::refresh);
    CanvasModelSlot(slot_CanvasModel_Fetch, &CanvasModelBroker::fetch);
    CanvasModelSlot(slot_CanvasModel_Take, &CanvasModelBroker::take);
    return true;
}

QUrl CanvasModelBroker::rootUrl()
{
    return model->rootUrl();
}

QModelIndex CanvasModelBroker::urlIndex(const QUrl &url)
{
    return model->index(url);
}

QUrl CanvasModelBroker::fileUrl(const QModelIndex &index)
{
    return model->fileUrl(index);
}

QList<QUrl> CanvasModelBroker::files()
{
    return model->files();
}

bool CanvasModelBroker::showHiddenFiles()
{
    return model->showHiddenFiles();
}

void CanvasModelBroker::setShowHiddenFiles(bool show)
{
    model->setShowHiddenFiles(show);
}

int CanvasModelBroker::sortOrder()
{
    return model->sortOrder();
}

void CanvasModelBroker::setSortOrder(int order)
{
    model->setSortOrder(static_cast<Qt::SortOrder>(order));
}

int CanvasModelBroker::sortRole()
{
    return model->sortRole();
}

void CanvasModelBroker::setSortRole(int role, int order)
{
    model->setSortRole(role, static_cast<Qt::SortOrder>(order));
}

QModelIndex CanvasModelBroker::index(int row)
{
    return model->index(row);
}

int CanvasModelBroker::rowCount()
{
    return model->rowCount(model->rootIndex());
}

QVariant CanvasModelBroker::data(const QUrl &url, int itemRole)
{
    return model->data(model->index(url), itemRole);
}

void CanvasModelBroker::sort()
{
    model->sort();
}

void CanvasModelBroker::refresh(bool global, int ms, bool updateFile)
{
    model->refresh(model->rootIndex(), global, ms, updateFile);
}

bool CanvasModelBroker::fetch(const QUrl &url)
{
    return model->fetch(url);
}

bool CanvasModelBroker::take(const QUrl &url)
{
    return model->take(url);
}
