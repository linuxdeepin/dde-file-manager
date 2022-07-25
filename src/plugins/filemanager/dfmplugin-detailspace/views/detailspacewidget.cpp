/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "detailspacewidget.h"
#include "detailview.h"

#include <QHBoxLayout>

using namespace dfmplugin_detailspace;

DetailSpaceWidget::DetailSpaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url)
{
    setCurrentUrl(url, 0);
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url, int widgetFilter)
{
    removeControls();
    detailSpaceUrl = url;
    detailView->setUrl(url, widgetFilter);
}

QUrl DetailSpaceWidget::currentUrl() const
{
    return detailSpaceUrl;
}

bool DetailSpaceWidget::insterExpandControl(const int &index, QWidget *widget)
{
    return detailView->insertCustomControl(index, widget);
}

void DetailSpaceWidget::removeControls()
{
    detailView->removeControl();
}

void DetailSpaceWidget::initializeUi()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::ColorRole::Base);

    QHBoxLayout *rvLayout = new QHBoxLayout(this);
    rvLayout->setMargin(0);
    detailView = new DetailView;
    QFrame *rightDetailVLine = new QFrame;
    rightDetailVLine->setFrameShape(QFrame::VLine);
    rvLayout->addWidget(rightDetailVLine);
    rvLayout->addWidget(detailView, 1);
}

void DetailSpaceWidget::initConnect()
{
}
