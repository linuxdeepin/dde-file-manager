/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef DFMBROWSEWINDOW_H
#define DFMBROWSEWINDOW_H

#include "displaydefaultview.h"
#include "displayviewlogic.h"
#include "addressbar.h"
#include "crumbbar.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm_filemanager_service_global.h"

#include <DMainWindow>
#include <DButtonBox>

#include <QPushButton>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_BEGIN_NAMESPACE

class BrowseWindowPrivate;
class BrowseWindow : public DMainWindow
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BrowseWindow)
    BrowseWindowPrivate* const d;
public:
    explicit BrowseWindow(QWidget *parent = nullptr);
    virtual ~BrowseWindow();
    void addView(const QString &scheme, DisplayViewLogic *viewWidget);
    bool viewIsAdded(const QString &scheme);
    void setRootUrl(const QUrl &url);
    void setSidebar(SideBar *sidebar);
    void setAddressBar(AddressBar *addressBar);
    void setCrumbBar(CrumbBar *crumbBar);
    void setNavBackButton(DButtonBoxButton *button);
    void setNavForwardButton(DButtonBoxButton *button);
    void setListViewButton(QToolButton *button);
    void setIconViewButton(QToolButton *button);
    void setSearchButton(QToolButton *button);
    void setSearchFilterButton(QToolButton *button);
    void setPropertyButton(QWidget *widget);
    void setPropertyView(QWidget *widget);
    SideBar* sidebar();
    AddressBar* addresssBar();
    CrumbBar *crumbBar();
    DButtonBoxButton * navBackButton();
    DButtonBoxButton* navForwardButton();
    QToolButton* listViewButton();
    QToolButton* iconViewButton();
    QToolButton *searchButton();
    QToolButton *searchFilterButton();
    QWidget* propertyButton();
    QWidget* propertyView();
};

DSB_FM_END_NAMESPACE

#endif // DFMBROWSEWINDOW_H
