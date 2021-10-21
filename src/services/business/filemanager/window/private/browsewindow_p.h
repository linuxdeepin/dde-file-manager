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
#ifndef BROWSEWINDOWPRIVATE_H
#define BROWSEWINDOWPRIVATE_H

//services
#include "window/browsewindow.h"
#include "window/browseview.h"
#include "window/navwidget.h"
#include "window/crumbbar.h"
#include "window/addressbar.h"
#include "window/optionbuttonbox.h"

//modules
#include "dfm-base/widgets/dfmsplitter/splitter.h"
#include "dfm-base/widgets/dfmsidebar/sidebar.h"

//DTK
#include <DTitlebar>
#include <DButtonBox>

//Qt
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>

DSB_FM_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE //use dtkwidget

/* @class BrowseWindowPrivate
 * @brief 文件背板扩展的主窗口二进制兼容类。
 */
class BrowseWindowPrivate : public QObject
{
    Q_OBJECT
    friend class BrowseWindow;
    BrowseWindow *const q;

    SideBar *sidebarIns = nullptr;           //左侧边栏

    QFrame *titleBarIns = nullptr;              //标题栏
    QHBoxLayout *titleBarLayoutIns = nullptr;   //标题栏布局

    NavWidget *navWidgetIns = nullptr;      //导航小部件
    AddressBar *addressBarIns = nullptr;     //地址編輯栏
    QToolButton *searchButtonIns = nullptr;     //搜索栏按钮
    QToolButton *searchFilterButtonIns = nullptr; //搜索过滤按钮
    OptionButtonBox * optionButtonBoxIns = nullptr; //功能按鈕栏
    CrumbBar *crumbBarIns = nullptr;         //面包屑

    DisplayDefaultView displayCheckViewIns;      //默认检查Url展示的View
    QWidget *displayWidgetIns = nullptr;            //居中組件背板
    QVBoxLayout *displayViewLayoutIns = nullptr;    //居中組件布局
    QHash<QString, DisplayViewLogic*> views{};  //展示的view类

    QWidget *propertyViewIns = nullptr;        //右侧属性界面
    Splitter *splitterIns = nullptr;        //布局

    BrowseView::ViewMode mode = BrowseView::IconMode;

public:
    explicit BrowseWindowPrivate(BrowseWindow *qq);

    virtual ~BrowseWindowPrivate();

    void setDefaultViewMode(BrowseView::ViewMode mode);

    void addView(const QString &scheme,
                 DisplayViewLogic *logic);

    bool viewIsAdded(const QString &scheme);

    DButtonBoxButton *navBackButton() const;
    void setNavBackButton(DButtonBoxButton *navBackButton);

    DButtonBoxButton *navForwardButton() const;
    void setNavForwardButton(DButtonBoxButton *navForwardButton);

    QToolButton *searchButton() const;
    void setSearchButton(QToolButton* searchButton);

    QToolButton *searchFilterButton() const;
    void setSearchFilterButton(QToolButton *searchFilterButton);

    OptionButtonBox *optionButtonBox() const;
    void setOptionButtonBox(OptionButtonBox *optionButtonBox);

    QToolButton *listViewButton() const;
    void setListViewButton(QToolButton *button);

    QToolButton *iconViewButton() const;
    void setIconViewButton(QToolButton *button);

    CrumbBar *crumbBar() const;
    void setCrumbBar(CrumbBar *crumbBar);

    QWidget *propertyView() const;
    void setPropertyView(QWidget *propertyView);

    SideBar *sidebar() const;
    void setSidebar(SideBar *sidebar);

    AddressBar *addressBar() const;
    void setAddressBar(AddressBar *addressBar);

private Q_SLOT:
    void doSearchButtonClicked(bool checked);
    void doViewModeButtonClicked(bool checked);
    void setRootUrl(const QUrl &url);

private:
    QWidget* currentDisplayView();
    void initDefaultLayout();    //初始化界面模块与布局
    void showAddrsssBar();    //switch addrasssBar and crumbBar show
    void showCrumbBar();
    void showSearchButton();
    void showSearchFilterButton();

    virtual bool eventFilter(QObject *watched, QEvent *event);
};

DSB_FM_END_NAMESPACE

#endif // BROWSEWINDOWPRIVATE_H
