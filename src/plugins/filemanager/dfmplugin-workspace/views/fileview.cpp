/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"
#include "models/filesortfilterproxymodel.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"
#include "baseitemdelegate.h"
#include "iconitemdelegate.h"
#include "listitemdelegate.h"
#include "statusbar.h"
#include "utils/workspacehelper.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "events/workspaceeventcaller.h"

#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FileView::FileView(const QUrl &url, QWidget *parent)
    : DListView(parent), d(new FileViewPrivate(this))
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    setTextElideMode(Qt::ElideMiddle);
    setAlternatingRowColors(false);
    setSelectionRectVisible(true);

    initializeModel();
    initializeDelegate();
    initializeStatusBar();
    initializeConnect();

    // TODO(liuyangming): init data from config
    QAbstractItemView::model()->sort(0);
    setRootUrl(url);
}

QWidget *FileView::widget() const
{
    return const_cast<FileView *>(this);
}

void FileView::setViewMode(ViewMode mode)
{
    setItemDelegate(d->delegates[static_cast<int>(mode)]);

    switch (mode) {
    case ViewMode::IconMode:
        setUniformItemSizes(false);
        setResizeMode(Adjust);
        setOrientation(QListView::LeftToRight, true);
        setSpacing(GlobalPrivate::kIconViewSpacing);

        d->initIconModeView();
        break;
    case ViewMode::ListMode:
        setUniformItemSizes(true);
        setResizeMode(Fixed);
        setOrientation(QListView::TopToBottom, false);
        setSpacing(GlobalPrivate::kListViewSpacing);

        if (model())
            setMinimumWidth(model()->columnCount() * GlobalPrivate::kListViewMinimumWidth);
        d->initListModeView();
        break;
    case ViewMode::ExtendMode:
        break;
    case ViewMode::AllViewMode:
        break;
    }
}

void FileView::setDelegate(ViewMode mode, BaseItemDelegate *view)
{
    if (!view)
        return;

    auto delegate = d->delegates[static_cast<int>(mode)];
    if (delegate) {
        if (delegate->parent())
            delegate->setParent(nullptr);
        delete delegate;
    }

    d->delegates[static_cast<int>(mode)] = view;
}

bool FileView::setRootUrl(const QUrl &url)
{
    model()->setRootUrl(url);

    loadViewState(url);

    delayUpdateStatusBar();
    setDefaultViewMode();

    return true;
}

QUrl FileView::rootUrl() const
{
    return model()->rootUrl();
}

AbstractBaseView::ViewState FileView::viewState() const
{
    // TODO(zhangs): return model state
    return AbstractBaseView::viewState();
}

QList<QAction *> FileView::toolBarActionList() const
{
    // TODO(zhangs): impl me
    return QList<QAction *>();
}

QList<QUrl> FileView::selectedUrlList() const
{
    // TODO(zhangs): impl me
    return QList<QUrl>();
}

void FileView::refresh()
{
    // TODO(zhangs): model()->refresh();
}

FileViewModel *FileView::model() const
{
    auto model = qobject_cast<FileSortFilterProxyModel *>(QAbstractItemView::model());
    if (model)
        return qobject_cast<FileViewModel *>(model->sourceModel());

    return nullptr;
}

void FileView::setModel(QAbstractItemModel *model)
{
    if (model->parent() != this)
        model->setParent(this);
    auto curr = FileView::model();
    if (curr)
        delete curr;
    DListView::setModel(model);
    QObject::connect(this, &FileView::clicked, this, &FileView::onClicked, Qt::UniqueConnection);
}

FileSortFilterProxyModel *FileView::proxyModel() const
{
    return qobject_cast<FileSortFilterProxyModel *>(QAbstractItemView::model());
}

int FileView::getColumnWidth(const int &column) const
{
    if (d->headerView)
        return d->headerView->sectionSize(column);

    return GlobalPrivate::kListViewDefaultWidth;
}

int FileView::getHeaderViewWidth() const
{
    if (d->headerView)
        return d->headerView->sectionsTotalWidth();

    return 0;
}

int FileView::selectedIndexCount() const
{
    return selectionModel()->selectedIndexes().count();
}

void FileView::onHeaderViewMouseReleased()
{
    if (d->headerView->sectionsTotalWidth() != width())
        d->allowedAdjustColumnSize = false;

    // TODO(liuyangming): save data to config
}

void FileView::onHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)

    // TODO(liuyangming): save data to config

    update();
}

void FileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    proxyModel()->setSortRole(model()->getRoleByColumn(logicalIndex));
    proxyModel()->sort(logicalIndex, order);

    //TODO liuyangming: save data to config
}

void FileView::onClicked(const QModelIndex &index)
{
    openIndexByClicked(ClickedAction::kClicked, index);
}

void FileView::onDoubleClicked(const QModelIndex &index)
{
    openIndexByClicked(ClickedAction::kDoubleClicked, index);
}

void FileView::keyPressEvent(QKeyEvent *event)
{
    // TODO(zhangs): impl me
    DListView::keyPressEvent(event);
}

void FileView::onScalingValueChanged(const int value)
{
    qobject_cast<IconItemDelegate *>(itemDelegate())->setIconSizeByIconSizeLevel(value);
}

void FileView::delayUpdateStatusBar()
{
    if (d->updateStatusBarTimer)
        d->updateStatusBarTimer->start();
}

void FileView::viewModeChanged(quint64 windowId, int viewMode)
{
    auto thisWindId = WorkspaceHelper::instance()->windowId(this);
    ViewMode mode = static_cast<ViewMode>(viewMode);
    if (thisWindId == windowId) {
        // TODO(yanghao): enum
        if (mode == ViewMode::IconMode) {
            setViewModeToIcon();
        } else if (mode == ViewMode::ListMode) {
            setViewModeToList();
        }
    }
}

void FileView::resizeEvent(QResizeEvent *event)
{
    if (d->headerView) {
        if (qAbs(d->headerView->sectionsTotalWidth() - width()) < 10)
            d->allowedAdjustColumnSize = true;

        d->updateListModeColumnWidth();
    }

    return DListView::resizeEvent(event);
}

void FileView::initializeModel()
{
    FileViewModel *model = new FileViewModel(this);
    FileSortFilterProxyModel *proxyModel = new FileSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    setModel(proxyModel);

    // TODO(liuyangming): refactor selection
    //    auto selectionModel = new FileSelectionModel(model);
    //    setSelectionModel(selectionModel);
}

void FileView::initializeDelegate()
{
    setDelegate(ViewMode::IconMode, new IconItemDelegate(this));
    setDelegate(ViewMode::ListMode, new ListItemDelegate(this));
}

void FileView::initializeStatusBar()
{
    d->statusBar = new StatusBar(this);
    d->statusBar->resetScalingSlider(GlobalPrivate::kIconSizeList.length() - 1);

    d->updateStatusBarTimer = new QTimer(this);
    d->updateStatusBarTimer->setInterval(100);
    d->updateStatusBarTimer->setSingleShot(true);

    addFooterWidget(d->statusBar);
}

void FileView::initializeConnect()
{
    connect(d->updateStatusBarTimer, &QTimer::timeout, this, &FileView::updateStatusBar);
    connect(d->statusBar->scalingSlider(), &QSlider::valueChanged, this, &FileView::onScalingValueChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::delayUpdateStatusBar);

    connect(this, &DListView::clicked, this, &FileView::onClicked);
    connect(this, &DListView::doubleClicked, this, &FileView::onDoubleClicked);
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::viewModeChanged, this, &FileView::viewModeChanged);
}

void FileView::updateStatusBar()
{
    int count = selectedIndexCount();
    if (count == 0) {
        d->statusBar->itemCounted(proxyModel()->rowCount());
        return;
    }

    QList<const FileViewItem *> list;
    for (const QModelIndex &index : selectedIndexes())
        list << sourceItem(index);

    d->statusBar->itemSelected(list);
}

void FileView::setDefaultViewMode()
{
    setViewMode(d->configViewMode);
}

void FileView::loadViewState(const QUrl &url)
{
    // TODO:(yanghao)
    QVariant defaultViewMode = Application::instance()->appAttribute(Application::kViewMode).toInt();
    d->configViewMode = static_cast<ViewMode>(fileViewStateValue(url, "viewMode", defaultViewMode).toInt());

    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    d->configIconSizeLevel = fileViewStateValue(url, "iconSizeLevel", defaultIconSize).toInt();
}

void FileView::openIndexByClicked(const ClickedAction action, const QModelIndex &index)
{
    ClickedAction configAction = ClickedAction::kDoubleClicked;
    if (action == configAction) {
        Qt::ItemFlags flags = model()->flags(proxyModel()->mapToSource(index));
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;

        //if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed()) TODO(liuyangming): remind code
        openIndex(index);
    }
}

void FileView::openIndex(const QModelIndex &index)
{
    const FileViewItem *item = sourceItem(index);

    if (!item)
        return;

    if (item->fileinfo()->isDir()) {
        auto windowID = WorkspaceHelper::instance()->windowId(this);
        WorkspaceEventCaller::sendChangeCurrentUrl(windowID, item->url());
    }
}

/**
 * @brief FileView::sourceItem get source FileViewItem by porxy index
 * @param index
 * @return
 */
const FileViewItem *FileView::sourceItem(const QModelIndex &index) const
{
    return model()->itemFromIndex(proxyModel()->mapToSource(index));
}

QVariant FileView::fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue)
{
    return Application::appObtuselySetting()->value("FileViewState", url).toMap().value(key, defalutValue);
}

void FileView::setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value)
{
    QVariantMap map = Application::appObtuselySetting()->value("FileViewState", url).toMap();

    map[key] = value;

    Application::appObtuselySetting()->setValue("FileViewState", url, map);
}
