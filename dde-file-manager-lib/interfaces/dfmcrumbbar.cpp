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

#include <QListWidget>

DFM_BEGIN_NAMESPACE

class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)

public:
    DFMCrumbBarPrivate(DFMCrumbBar *qq);

    QListWidget crumbListWidget;

    DFMCrumbBar *q_ptr = nullptr;

    void clearCrumbs();
    void addCrumb(DFMCrumbItem* item);
};

DFMCrumbBarPrivate::DFMCrumbBarPrivate(DFMCrumbBar *qq)
{
    Q_UNUSED(qq);
}

void DFMCrumbBarPrivate::clearCrumbs()
{
    qWarning("DFMCrumbBarPrivate::clearCrumbs() may need implement!!!");
    crumbListWidget.clear();
}

void DFMCrumbBarPrivate::addCrumb(DFMCrumbItem *item)
{
    Q_UNUSED(item);
}


DFMCrumbBar::DFMCrumbBar(QWidget *parent) : QWidget(parent)
{

}

DFMCrumbBar::~DFMCrumbBar()
{

}

void DFMCrumbBar::updateCrumbs(const DUrl &url)
{
    Q_UNUSED(url);
}

DFM_END_NAMESPACE
