// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarimageviewmodel.h"
#include "docsheet.h"
#include "pagerenderthread.h"

#include <QApplication>
#include <QTimer>
#include <QtDebug>

using namespace plugin_filepreview;
ImagePageInfo_t::ImagePageInfo_t(int index)
    : pageIndex(index)
{
}

bool ImagePageInfo_t::operator==(const ImagePageInfo_t &other) const
{
    return (this->pageIndex == other.pageIndex);
}

bool ImagePageInfo_t::operator<(const ImagePageInfo_t &other) const
{
    return (this->pageIndex < other.pageIndex);
}

bool ImagePageInfo_t::operator>(const ImagePageInfo_t &other) const
{
    return (this->pageIndex > other.pageIndex);
}

SideBarImageViewModel::SideBarImageViewModel(DocSheet *sheet, QObject *parent)
    : QAbstractListModel(parent), parentObj(parent), docSheet(sheet)
{
    connect(docSheet, &DocSheet::sigPageModified, this, &SideBarImageViewModel::onUpdateImage);
}

void SideBarImageViewModel::resetData()
{
    beginResetModel();
    pageList.clear();
    endResetModel();
}

void SideBarImageViewModel::initModelLst(const QList<ImagePageInfo_t> &pagelst, bool sort)
{
    beginResetModel();

    pageList = pagelst;

    if (sort)
        std::sort(pageList.begin(), pageList.end());

    endResetModel();
}

void SideBarImageViewModel::changeModelData(const QList<ImagePageInfo_t> &pagelst)
{
    pageList = pagelst;
}

int SideBarImageViewModel::rowCount(const QModelIndex &) const
{
    return pageList.size();
}

int SideBarImageViewModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant SideBarImageViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int nRow = pageList.at(index.row()).pageIndex;

    if (-1 == nRow)
        return QVariant();

    if (role == ImageinfoType_e::IMAGE_PIXMAP) {
        QPixmap pixmap = docSheet->thumbnail(nRow);

        if (pixmap.isNull()) {
            //! 先填充空白
            QPixmap emptyPixmap(200, 200);
            emptyPixmap.fill(Qt::white);
            docSheet->setThumbnail(nRow, emptyPixmap);

            //! 使用线程
            DocPageThumbnailTask task;
            task.sheet = docSheet;
            task.index = nRow;
            task.model = const_cast<SideBarImageViewModel *>(this);
            PageRenderThread::appendTask(task);
        }

        return QVariant::fromValue(pixmap);
    } else if (role == ImageinfoType_e::IMAGE_ROTATE) {
        return QVariant::fromValue(docSheet->operation().rotation * 90);
    } else if (role == Qt::AccessibleTextRole) {
        return index.row();
    } else if (role == ImageinfoType_e::IMAGE_PAGE_SIZE) {
        return QVariant::fromValue(docSheet->pageSizeByIndex(nRow));
    }
    return QVariant();
}

bool SideBarImageViewModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
    if (!index.isValid())
        return false;
    return QAbstractListModel::setData(index, data, role);
}

QList<QModelIndex> SideBarImageViewModel::getModelIndexForPageIndex(int pageIndex)
{
    QList<QModelIndex> modelIndexlst;

    int pageSize = pageList.size();

    for (int index = 0; index < pageSize; index++) {
        if (pageList.at(index) == ImagePageInfo_t(pageIndex))
            modelIndexlst << this->index(index);
    }
    return modelIndexlst;
}

int SideBarImageViewModel::getPageIndexForModelIndex(int row)
{
    if (row >= 0 && row < pageList.size())
        return pageList.at(row).pageIndex;
    return -1;
}

void SideBarImageViewModel::onUpdateImage(int index)
{
    DocPageThumbnailTask task;
    task.sheet = docSheet;
    task.index = index;
    task.model = const_cast<SideBarImageViewModel *>(this);
    PageRenderThread::appendTask(task);
}

void SideBarImageViewModel::handleRenderThumbnail(int index, QPixmap pixmap)
{
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    docSheet->setThumbnail(index, pixmap);

    const QList<QModelIndex> &modelIndexlst = getModelIndexForPageIndex(index);
    for (const QModelIndex &modelIndex : modelIndexlst)
        emit dataChanged(modelIndex, modelIndex);
}
