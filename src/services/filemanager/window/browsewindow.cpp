/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "private/browsewindow_p.h"
#include "browsewindow.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/widgets/dfmsplitter/splitter.h"

#include <DSplitter>
#include <DTitlebar>

#include <QLabel>
#include <QVBoxLayout>

DSB_FM_BEGIN_NAMESPACE

/* @class DFMBrowseWindow
 * @brief 文件背板扩展的主窗口。
 *  该类只提供布局不会涉及实体对象的new，除非固定的业务逻辑如
 *  DFMDisplayViewDefault 默认展示的view，这将会提供以错
 *  误信息提示的View，一个window可以同时存在：
 *      1.多个展示的view
 *      2.一个地址编辑栏
 *      3.一个面包屑
 *      4.一个导航后退按钮
 *      5.一个导航前进按钮
 */
BrowseWindow::BrowseWindow(QWidget *parent)
    : DMainWindow(parent), d(new BrowseWindowPrivate(this))
{
}

BrowseWindow::~BrowseWindow()
{
}

//使用scheme关联进行view的添加
void BrowseWindow::addView(const QString &scheme,
                           DisplayViewLogic *viewWidget)
{
    if (viewWidget) {
        d->addView(scheme, viewWidget);
    } else {
        qWarning() << "Called addview method argument class "
                      "DFMDisplayViewLogic can't show,"
                      " cause type not is widget";
    }
}

//判断当前scheme是否关联view
bool BrowseWindow::viewIsAdded(const QString &scheme)
{
    return d->viewIsAdded(scheme);
}

//设置左侧边栏
void BrowseWindow::setSidebar(SideBar *sidebar)
{
    return d->setSidebar(sidebar);
}

//获取左侧边栏
SideBar *BrowseWindow::sidebar()
{
    return d->sidebar();
}

//设置地址栏
void BrowseWindow::setAddressBar(AddressBar *addressBar)
{
    return d->setAddressBar(addressBar);
}

//获取地址栏
AddressBar *BrowseWindow::addresssBar()
{
    return d->addressBar();
}

//设置面包屑
void BrowseWindow::setCrumbBar(CrumbBar *crumbBar)
{
    return d->setCrumbBar(crumbBar);
}

//获取面包屑
CrumbBar *BrowseWindow::crumbBar()
{
    return d->crumbBar();
}

//设置导航后退按钮
void BrowseWindow::setNavBackButton(DButtonBoxButton *button)
{
    return d->setNavBackButton(button);
}

//设置导航前进按钮
DButtonBoxButton *BrowseWindow::navBackButton()
{
    return d->navBackButton();
}

//设置导航前进按钮
void BrowseWindow::setNavForwardButton(DButtonBoxButton *button)
{
    return d->setNavForwardButton(button);
}

//获取导航前进按钮
DButtonBoxButton *BrowseWindow::navForwardButton()
{
    return d->navForwardButton();
}

//设置listview点击button
void BrowseWindow::setListViewButton(QToolButton *button)
{
    return d->setListViewButton(button);
}

//获取listview点击button
QToolButton *BrowseWindow::listViewButton()
{
    return d->listViewButton();
}

//设置iconview点击的button
void BrowseWindow::setIconViewButton(QToolButton *button)
{
    return d->setIconViewButton(button);
}

//获取iconview点击的button
QToolButton *BrowseWindow::iconViewButton()
{
    return d->iconViewButton();
}

//设置搜索按钮
void BrowseWindow::setSearchButton(QToolButton *button)
{
    return d->setSearchButton(button);
}

//获取搜索按钮
QToolButton *BrowseWindow::searchButton()
{
    return d->searchButton();
}

//设置搜索过滤按钮
void BrowseWindow::setSearchFilterButton(QToolButton *button)
{
    return d->setSearchFilterButton(button);
}

//索取搜索过滤按钮
QToolButton *BrowseWindow::searchFilterButton()
{
    return d->searchFilterButton();
}

//设置属性界面点击的button
void BrowseWindow::setPropertyButton(QToolButton *button)
{
    d->setDetailButton(button);
}

//获取属性界面点击的button
QToolButton *BrowseWindow::propertyButton()
{
    return d->detailButton();
}

//设置属性界面
void BrowseWindow::setPropertyView(DetailView *widget)
{
    return d->setPropertyView(widget);
}

//获取属性界面
DetailView *BrowseWindow::propertyView()
{
    return d->propertyView();
}

//设置当前window展示的url
void BrowseWindow::setRootUrl(const QUrl &url)
{
    //內部存在业务逻辑所以需要兼容到private
    return d->setRootUrl(url);
}

// 获取当前window展示的url
const QUrl BrowseWindow::rootUrl()
{
    return d->rootUrl();
}

DSB_FM_END_NAMESPACE
