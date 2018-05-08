/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbbar.h"
#include "dfmcrumbitem.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>

#include <QDebug>

DFM_BEGIN_NAMESPACE

class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)

public:
    DFMCrumbBarPrivate(DFMCrumbBar *qq);

    QPushButton leftArrow;
    QPushButton rightArrow;
    QScrollArea crumbListScrollArea;
    QWidget *crumbListHolder;
    QHBoxLayout *crumbListLayout;
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;

    DFMCrumbBar *q_ptr = nullptr;

    void clearCrumbs();
    void addCrumb(DFMCrumbItem* item);

private:
    void initUI();
};

DFMCrumbBarPrivate::DFMCrumbBarPrivate(DFMCrumbBar *qq)
    : q_ptr(qq)
{
    initUI();
    // test
    addCrumb(new DFMCrumbItem(DUrl::fromComputerFile("/")));
    addCrumb(new DFMCrumbItem(DUrl::fromComputerFile("/")));
    addCrumb(new DFMCrumbItem(DUrl::fromComputerFile("/")));
}

void DFMCrumbBarPrivate::clearCrumbs()
{
    qWarning("DFMCrumbBarPrivate::clearCrumbs() may need implement!!!");
}

void DFMCrumbBarPrivate::addCrumb(DFMCrumbItem *item)
{
    crumbListLayout->addWidget(item);
    crumbListHolder->adjustSize();
}

void DFMCrumbBarPrivate::initUI()
{
    Q_Q(DFMCrumbBar);

    // Crumbbar Widget
    //q->setObjectName("DCrumbWidget");
    q->setFixedHeight(24);
    q->setObjectName("DCrumbBackgroundWidget");

    // Arrows
    leftArrow.setObjectName("backButton");
    leftArrow.setFixedWidth(26);
    leftArrow.setFixedHeight(24);
    leftArrow.setFocusPolicy(Qt::NoFocus);
    rightArrow.setObjectName("forwardButton");
    rightArrow.setFixedWidth(26);
    rightArrow.setFixedHeight(24);
    rightArrow.setFocusPolicy(Qt::NoFocus);
    leftArrow.hide();
    rightArrow.hide();

    // Crumb List Layout
    crumbListScrollArea.setObjectName("DCrumbListScrollArea");
    crumbListScrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListScrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListScrollArea.setFocusPolicy(Qt::NoFocus);
    crumbListScrollArea.setContentsMargins(0,0,0,0);

    crumbListHolder = new QWidget();
    //crumbListHolder->setStyleSheet("background: blue");
    crumbListHolder->setContentsMargins(0,0,30,0); // right 30 for easier click
    crumbListHolder->setFixedHeight(q->height());
    crumbListScrollArea.setWidget(crumbListHolder);

    crumbListLayout = new QHBoxLayout;
    crumbListLayout->setMargin(0);
    crumbListLayout->setSpacing(0);
    crumbListLayout->setAlignment(Qt::AlignLeft);
//    crumbListLayout->setSizeConstraint(QLayout::SetMaximumSize);
    crumbListLayout->setContentsMargins(0, 0, 0, 0);
    crumbListHolder->setLayout(crumbListLayout);

    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout;
    crumbBarLayout->addWidget(&leftArrow);
    crumbBarLayout->addWidget(&crumbListScrollArea);
    crumbBarLayout->addWidget(&rightArrow);
    crumbBarLayout->setContentsMargins(0,0,0,0);
    crumbBarLayout->setSpacing(0);
    q->setLayout(crumbBarLayout);
}

DFMCrumbBar::DFMCrumbBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new DFMCrumbBarPrivate(this))
{

}

DFMCrumbBar::~DFMCrumbBar()
{

}

void DFMCrumbBar::updateCrumbs(const DUrl &url)
{
    Q_UNUSED(url);
}

void DFMCrumbBar::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);
    d->clickedPos = event->globalPos();

    QWidget::mousePressEvent(event);
}

void DFMCrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);

    //blumia: no need to check if it's clicked on other widgets
    //        since this will only happend when clicking empty.
    if (d->clickedPos == event->globalPos()) {
        emit toggleSearchBar();
    }

    QWidget::mouseReleaseEvent(event);
}

DFM_END_NAMESPACE
