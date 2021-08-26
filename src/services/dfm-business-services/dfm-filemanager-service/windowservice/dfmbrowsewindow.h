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
#ifndef DFMBROWSEWINDOW_H
#define DFMBROWSEWINDOW_H

#include "dfmdisplaydefaultview.h"
#include "dfmdisplayviewlogic.h"

#include <DMainWindow>
#include <DButtonBox>

#include <QPushButton>

DWIDGET_USE_NAMESPACE

class DFMBrowseWindowPrivate;
class DFMAddressBar;
class DFMSideBar;
class DFMCrumbBar;
class QToolButton;

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
class DFMBrowseWindow : public DMainWindow
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMBrowseWindow)
    DFMBrowseWindowPrivate * const d;

public:
    explicit DFMBrowseWindow(QWidget *parent = nullptr);

    //使用scheme关联进行view的添加
    void addView(const QString &scheme,
                 DFMDisplayViewLogic *viewWidget);

    //判断当前scheme是否关联view
    bool viewIsAdded(const QString &scheme);

    //设置左侧边栏
    void setSidebar(DFMSideBar *sidebar);

    //获取左侧边栏
    DFMSideBar* sidebar();

    //设置地址栏
    void setAddressBar(DFMAddressBar *addressBar);

    //获取地址栏
    DFMAddressBar* addresssBar();

    //设置面包屑
    void setCrumbBar(DFMCrumbBar *crumbBar);

    //获取面包屑
    DFMCrumbBar *crumbBar();

    //设置导航后退按钮
    void setNavBackButton(DButtonBoxButton *button);

    //设置导航前进按钮
    DButtonBoxButton * navBackButton();

    //设置导航前进按钮
    void setNavForwardButton(DButtonBoxButton *button);

    //获取导航前进按钮
    DButtonBoxButton* navForwardButton();

    //设置listview点击button
    void setListViewButton(QToolButton *button);

    //获取listview点击button
    QToolButton* listViewButton();

    //设置iconview点击的button
    void setIconViewButton(QToolButton *button);

    //获取iconview点击的button
    QToolButton* iconViewButton();

    //设置搜索按钮
    void setSearchButton(QToolButton *button);

    //获取搜索按钮
    QToolButton *searchButton();

    //设置搜索过滤按钮
    void setSearchFilterButton(QToolButton *button);

    //索取搜索过滤按钮
    QToolButton *searchFilterButton();

    //设置属性界面点击的button
    void setPropertyButton();

    //获取属性界面点击的button
    QWidget* propertyButton();

    //设置属性界面
    void setPropertyView();

    //获取属性界面
    QWidget* propertyView();

    //设置当前window展示的url
    void setRootUrl(const QUrl &url);


};

#endif // DFMBROWSEWINDOW_H
