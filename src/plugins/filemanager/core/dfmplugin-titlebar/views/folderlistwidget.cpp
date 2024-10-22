// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/folderlistwidget_p.h"
#include "views/folderlistwidget.h"
#include "views/folderviewdelegate.h"

#include <dfm-base/base/schemefactory.h>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

static constexpr int kMaxFolderCount = 8;
static constexpr int kFolderBatchSize = 2000;
static constexpr int kFolderListItemMargin { 6 };

FolderListWidgetPrivate::FolderListWidgetPrivate(FolderListWidget *qq)
    : QObject(qq), q(qq)
{
    q->resize(172, kItemMargin * 2 + kFolderItemHeight * 8);
    q->setMouseTracking(true);
    layout = new QVBoxLayout(qq);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    folderModel = new QStandardItemModel(this);
    folderView = new DListView(qq);
    folderView->setMouseTracking(true);
    folderView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    folderView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    folderView->setViewportMargins(kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin, kFolderListItemMargin);
    folderView->setUniformItemSizes(true);
    folderView->setViewMode(QListView::ListMode);
    folderView->setResizeMode(QListView::Adjust);
    folderView->setMovement(QListView::Static);
    folderView->setLayoutMode(QListView::Batched);
    folderView->setBatchSize(kFolderBatchSize);

    folderView->setModel(folderModel);
    folderDelegate = new FolderViewDelegate(folderView);
    folderView->setItemDelegate(folderDelegate);

    layout->addWidget(folderView);
    q->setLayout(layout);

    initConnect();
}

FolderListWidgetPrivate::~FolderListWidgetPrivate()
{
}

void FolderListWidgetPrivate::initConnect()
{
    connect(folderView, &DListView::clicked, this, &FolderListWidgetPrivate::clicked);
}

void FolderListWidgetPrivate::clicked(const QModelIndex &index)
{
    q->hide();

    if (index.isValid()) {
        int row = index.row();
        if (row >= 0 && row < crumbDatas.size()) {
            Q_EMIT q->urlButtonActivated(crumbDatas[row].url);
        }
    }
}

void FolderListWidgetPrivate::returnPressed()
{
    if (!folderView) {
        q->hide();
        return;
    }
    clicked(folderView->currentIndex());
}

void FolderListWidgetPrivate::selectUp()
{
    if (!folderView) {
        q->hide();
        return;
    }
    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(folderModel->rowCount() - 1, 0);
    } else {
        int row = curIndex.row() - 1;
        if (row < 0)
            row = folderModel->rowCount() - 1;
        curIndex = folderModel->index(row, 0);
    }
    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

void FolderListWidgetPrivate::selectDown()
{
    if (!folderView) {
        q->hide();
        return;
    }
    auto curIndex = folderView->currentIndex();
    if (!curIndex.isValid()) {
        if (folderModel->rowCount() > 0)
            curIndex = folderModel->index(0, 0);
    } else {
        int row = curIndex.row() + 1;
        if (row >= folderModel->rowCount())
            row = 0;
        curIndex = folderModel->index(row, 0);
    }
    if (curIndex.isValid())
        folderView->setCurrentIndex(curIndex);
}

FolderListWidget::FolderListWidget(QWidget *parent)
    : DBlurEffectWidget(parent), d(new FolderListWidgetPrivate(this))
{
    setWindowFlag(Qt::Popup);
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);
}

FolderListWidget::~FolderListWidget() = default;

void FolderListWidget::setFolderList(const QList<CrumbData> &datas)
{
    d->folderModel->clear();
    d->crumbDatas = datas;
    int dataNum = 0;
    for (auto data : datas) {
        auto info = InfoFactory::create<FileInfo>(data.url);
        if (!info.isNull()) {
            QStandardItem *item = new QStandardItem(info->fileIcon(), data.displayText);
            d->folderModel->insertRow(dataNum, item);
            dataNum++;
        }
    }
    int folderCount = datas.size() > kMaxFolderCount ? kMaxFolderCount : datas.size();
    if (datas.size() > 1) {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin, kItemMargin, kItemMargin);
        setFixedHeight(kItemMargin * 2 + kFolderItemHeight * folderCount);
    } else {
        d->folderView->setViewportMargins(kItemMargin, kItemMargin * 3 / 2, kItemMargin, kItemMargin * 3 / 2);
        setFixedHeight(kItemMargin * 3 + kFolderItemHeight * folderCount);
    }
}

void FolderListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        d->selectUp();
    } else if (event->key() == Qt::Key_Down) {
        d->selectDown();
    } else if (event->key() == Qt::Key_Return) {
        d->returnPressed();
    }
    DBlurEffectWidget::keyPressEvent(event);
}

void FolderListWidget::hideEvent(QHideEvent *event)
{
    emit hidden();
    DBlurEffectWidget::hideEvent(event);
}
