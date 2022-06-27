/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "sidebarimagelistview.h"
#include "docsheet.h"
#include "sidebarimageviewmodel.h"

#include <DGuiApplicationHelper>

#include <QApplication>
#include <QScroller>
#include <QScrollBar>
#include <QSet>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
SideBarImageListView::SideBarImageListView(DocSheet *sheet, QWidget *parent)
    : DListView(parent), docSheet(sheet)
{
    initControl();
    setAutoScroll(false);
    setProperty("adaptScale", 0.5);
    setSpacing(0);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeMode(QListView::Fixed);
    setViewMode(QListView::ListMode);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setAutoFillBackground(true);
    QPalette palet = this->palette();
    palet.setColor(QPalette::Background, Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().itemBackground().color());
    setPalette(palet);
}

void SideBarImageListView::initControl()
{
    listType = E_SideBar::SIDE_THUMBNIL;
    imageModel = new SideBarImageViewModel(docSheet, this);
    this->setModel(imageModel);
}

void SideBarImageListView::handleOpenSuccess()
{
    if (listType == E_SideBar::SIDE_THUMBNIL) {

        QList<ImagePageInfo_t> pageSrclst;
        int pagesNum = docSheet->pageCount();
        for (int index = 0; index < pagesNum; index++)
            pageSrclst << ImagePageInfo_t(index);
        imageModel->initModelLst(pageSrclst);
    }
}

void SideBarImageListView::onItemClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        docSheet->jumpToIndex(imageModel->getPageIndexForModelIndex(index.row()));
        emit sigListItemClicked(index.row());
    }
}

bool SideBarImageListView::scrollToIndex(int index, bool scrollTo)
{
    const QList<QModelIndex> &indexlst = imageModel->getModelIndexForPageIndex(index);
    if (indexlst.size() > 0) {
        const QModelIndex &index = indexlst.first();
        if (scrollTo)
            this->scrollTo(index);
        this->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        this->setCurrentIndex(index);
        return true;
    } else {
        this->setCurrentIndex(QModelIndex());
        this->clearSelection();
        return false;
    }
}

void SideBarImageListView::mousePressEvent(QMouseEvent *event)
{
    DListView::mousePressEvent(event);
    onItemClicked(this->indexAt(event->pos()));
}

QModelIndex SideBarImageListView::pageUpIndex()
{
    return DListView::moveCursor(QAbstractItemView::MovePageUp, Qt::NoModifier);
}

QModelIndex SideBarImageListView::pageDownIndex()
{
    return DListView::moveCursor(QAbstractItemView::MovePageDown, Qt::NoModifier);
}
