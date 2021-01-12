/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "extendview.h"
#include <QListWidget>
#include <QHBoxLayout>
#include <QDir>
#include <QDebug>
#include "ddetailview.h"
#include "dfileview.h"


ExtendView::ExtendView(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}

ExtendView::~ExtendView()
{

}

void ExtendView::initUI()
{
    m_extendListView = new QListWidget;
    m_extendListView->setFlow(QListWidget::LeftToRight);
    m_detailView = new DDetailView;
    m_detailView->setFixedWidth(200);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_extendListView);
    mainLayout->addWidget(m_detailView);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void ExtendView::initConnect()
{

}

void ExtendView::setStartUrl(const DUrl &url)
{
    if (url == m_currentUrl) {
        return;
    }
//    m_extendListView->clear();
//    DUrlList urls = m_startUrl.childrenList(url);

//    foreach (DUrl url, urls) {
//        qDebug()<< url;
//        QListWidgetItem* item = new QListWidgetItem;
//        item->setSizeHint(QSize(200, 10));
//        m_extendListView->addItem(item);

//        DFileView* view = new DFileView;
//        view->setViewModeToExtend();
//        view->setFixedWidth(200);
//        view->setCurrentUrl(url);
//        m_extendListView->setItemWidget(item, view);
//    }

    m_currentUrl = url;
}

