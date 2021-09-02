/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "private/browsewindow_p.h"
#include "browsewindow.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/dfmsplitter/splitter.h"

#include <DSplitter>
#include <DTitlebar>

#include <QLabel>
#include <QVBoxLayout>

DSB_FM_BEGIN_NAMESPACE

DFMBrowseWindow::DFMBrowseWindow(QWidget *parent)
    : DMainWindow (parent)
    , d(new DFMBrowseWindowPrivate(this))
{

}

void DFMBrowseWindow::addView(const QString &scheme,
                              DFMDisplayViewLogic* viewWidget)
{
    if(viewWidget) {
        d->addview(scheme,viewWidget);
    } else {
        qWarning() << "Called addview method argument class "
                      "DFMDisplayViewLogic can't show,"
                      " cause type not is widget";
    }
}

bool DFMBrowseWindow::viewIsAdded(const QString &scheme)
{
    return d->viewIsAdded(scheme);
}

void DFMBrowseWindow::setSidebar(DFMSideBar *sidebar)
{
    return d->setSidebar(sidebar);
}

DFMSideBar *DFMBrowseWindow::sidebar()
{
    return d->sidebar();
}

void DFMBrowseWindow::setAddressBar(DFMAddressBar *addressBar)
{
    return d->setAddressBar(addressBar);
}

DFMAddressBar *DFMBrowseWindow::addresssBar()
{
    return d->addressBar();
}

void DFMBrowseWindow::setCrumbBar(DFMCrumbBar *crumbBar)
{
    return d->setCrumbBar(crumbBar);
}

DFMCrumbBar *DFMBrowseWindow::crumbBar()
{
    return d->crumbBar();
}

void DFMBrowseWindow::setNavBackButton(DButtonBoxButton *button)
{
   return d->setNavBackButton(button);
}

DButtonBoxButton *DFMBrowseWindow::navBackButton()
{
    return d->navBackButton();
}

void DFMBrowseWindow::setNavForwardButton(DButtonBoxButton *button)
{
    return d->setNavForwardButton(button);
}

DButtonBoxButton *DFMBrowseWindow::navForwardButton()
{
    return d->navForwardButton();
}

void DFMBrowseWindow::setListViewButton(QToolButton *button)
{
    return d->setListViewButton(button);
}

QToolButton *DFMBrowseWindow::listViewButton()
{
    return d->listViewButton();
}

void DFMBrowseWindow::setIconViewButton(QToolButton *button)
{
    return d->setIconViewButton(button);
}

QToolButton *DFMBrowseWindow::iconViewButton()
{
    return d->iconViewButton();
}

void DFMBrowseWindow::setSearchButton(QToolButton *button)
{
    return d->setSearchButton(button);
}

QToolButton *DFMBrowseWindow::searchButton()
{
    return d->searchButton();
}

void DFMBrowseWindow::setSearchFilterButton(QToolButton *button)
{
    return d->setSearchFilterButton(button);
}

QToolButton *DFMBrowseWindow::searchFilterButton()
{
    return d->searchFilterButton();
}

void DFMBrowseWindow::setRootUrl(const QUrl &url)
{
    //內部存在业务逻辑所以需要兼容到private
    return d->setRootUrl(url);
}

DSB_FM_END_NAMESPACE
