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

#include "fileview_p.h"
#include "views/headerview.h"
#include "views/statusbar.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewPrivate::FileViewPrivate(FileView *qq)
    : q(qq)
{
}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    int contentWidth = q->maximumViewportSize().width();
    return qMax((contentWidth - 1) / itemWidth, 1);
}

QUrl FileViewPrivate::modelIndexUrl(const QModelIndex &index) const
{
    return index.data().toUrl();
}

void FileViewPrivate::initIconModeView()
{
    if (headerView) {
        headerView->disconnect();
        q->removeHeaderWidget(0);
        delete headerView;
        headerView = nullptr;
    }

    if (statusBar) {
        statusBar->setScalingVisible(true);
        q->setIconSize(QSize(kIconSizeList[currentIconSizeLevel],
                             kIconSizeList[currentIconSizeLevel]));
        statusBar->scalingSlider()->setValue(currentIconSizeLevel);
    }
}

void FileViewPrivate::initListModeView()
{
    if (!headerView) {
        headerView = new HeaderView(Qt::Orientation::Horizontal, q);

        headerView->setHighlightSections(false);
        headerView->setSectionsClickable(true);
        headerView->setSortIndicatorShown(true);
        headerView->setSectionResizeMode(QHeaderView::Interactive);
        headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        headerView->setCascadingSectionResizes(false);
        headerView->setFixedHeight(GlobalPrivate::kListViewIconSize);
        headerView->setMinimumSectionSize(GlobalPrivate::kListViewMinimumWidth);

        headerView->setModel(q->model());
        if (q->selectionModel()) {
            headerView->setSelectionModel(q->selectionModel());
        }
    }

    q->addHeaderWidget(headerView);

    QObject::connect(headerView, &HeaderView::mouseReleased, q, &FileView::onHeaderViewMouseReleased);
    QObject::connect(headerView, &HeaderView::sectionResized, q, &FileView::onHeaderSectionResized);
    QObject::connect(headerView, &HeaderView::sortIndicatorChanged, q, &FileView::onSortIndicatorChanged);

    q->setIconSize(QSize(GlobalPrivate::kListViewIconSize, GlobalPrivate::kListViewIconSize));
    updateListModeColumnWidth();

    if (statusBar)
        statusBar->setScalingVisible(false);
}

void FileViewPrivate::updateListModeColumnWidth()
{
    if (!allowedAdjustColumnSize)
        return;
    if (!headerView)
        return;

    int nameColumnWidth = q->width();
    for (int i = 1; i < headerView->model()->columnCount(); ++i) {
        int columnWidth = q->model()->getColumnWidth(i);
        headerView->resizeSection(i, columnWidth);
        nameColumnWidth -= columnWidth;
    }
    headerView->resizeSection(0, nameColumnWidth);
}

bool FileViewPrivate::normalKeyPressEventHandle(const QKeyEvent *event)
{
    const QList<QUrl> &urls = q->selectedUrlList();
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (!urls.isEmpty()) {
            // Todo(yanghao):editingIndex handle
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceEventCaller::sendOpenFiles(windowID, urls);
            return true;
        }
        break;
    case Qt::Key_Backspace: {
        cdUp();
        return true;
    }
    case Qt::Key_Delete: {
        // Todo(yanghao): QUrl to LocalFile
        // complete deletion eg: gvfs, vault
        // only support trash on root url
        if (!urls.isEmpty()) {
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceEventCaller::sendMoveToTrash(windowID, urls);
            return true;
        }
        break;
    }
    case Qt::Key_End: {
        if (urls.isEmpty()) {
            auto index = q->model()->index(q->count() - 1, 0);
            q->setCurrentIndex(index);
        }
        return true;
    }
    default:
        break;
    }

    return false;
}

bool FileViewPrivate::cdUp()
{
    // Todo(yanghao):
    const QUrl &oldCurrentUrl = q->rootUrl();
    QUrl parentUrl = UrlRoute::urlParent(oldCurrentUrl);

    if (parentUrl.isValid()) {
        auto windowID = WorkspaceHelper::instance()->windowId(q);
        WorkspaceHelper::instance()->actionOpen(windowID, { parentUrl });
        return true;
    }
    return false;
}

bool FileViewPrivate::processKeyPressEvent(QKeyEvent *event)
{
    const QList<QUrl> &urls = q->selectedUrlList();

    switch (event->modifiers()) {
    case Qt::NoModifier:
        if (event->key() == Qt::Key_Space) {
            // Todo(yanghao): sorted urls
            WorkspaceHelper::instance()->actionShowFilePreviewDialog(urls);
            return true;
        } else if (normalKeyPressEventHandle(event)) {
            return true;
        }
        break;
    case Qt::KeypadModifier:
        if (normalKeyPressEventHandle(event)) {
            return true;
        }
        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_H: {
            // Todo(yanghao): preSelectionUrls
            // d->preSelectionUrls = urls;
            // q->itemDelegate()->hideAllIIndexWidget();
            q->clearSelection();
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionHiddenFiles(windowID, q->rootUrl());
            return true;
        }
        case Qt::Key_I: {
            // ToDo(yanghao): network not support
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionProperty(windowID, urls);
            return true;
        }
        case Qt::Key_Up: {
            cdUp();
            return true;
        }
        case Qt::Key_Down: {
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionOpen(windowID, urls);
            return true;
        }
        default:
            break;
        }
        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (urls.isEmpty())
                return true;

            // ToDo(yanghao):only support trash on root url
            // 共享文件夹不用弹出彻底删除对话框
            // 网络邻居目录不用弹出彻底删除对话框
            // 最近使用目录不用弹出彻底删除对话框
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionDeleteFiles(windowID, urls);
            return true;
        } else if (event->key() == Qt::Key_T) {
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionOpenInTerminal(windowID, urls);
            return true;
        }
        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_N) {
            // Todo(yanghao):editingIndex handle
            q->clearSelection();
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionNewFolder(windowID, q->rootUrl());
            return true;
        }
        break;
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Up:
            cdUp();
            return true;
        case Qt::Key_Down: {
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionOpen(windowID, urls);
            return true;
        }
        case Qt::Key_Home:
            auto windowID = WorkspaceHelper::instance()->windowId(q);
            WorkspaceHelper::instance()->actionOpen(windowID, urls);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}
