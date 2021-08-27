
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
#ifndef DFMBROWSEWINDOWPRIVATE_H
#define DFMBROWSEWINDOWPRIVATE_H

//services
#include "windowservice/browsewindow.h"
#include "windowservice/browseview.h"
#include "windowservice/navwidget.h"
#include "windowservice/crumbbar.h"
#include "windowservice/addressbar.h"
#include "windowservice/optionbuttonbox.h"

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

/* @class DFMBrowseWindowPrivate
 * @brief 文件背板扩展的主窗口二进制兼容类。
 */
class DFMBrowseWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DFMBrowseWindow)
    DFMBrowseWindow * const q_ptr;

    DFMSideBar* m_sidebar = nullptr;           //左侧边栏

    QFrame *m_titleBar = nullptr;              //标题栏
    QHBoxLayout *m_titleBarLayout = nullptr;   //标题栏布局

    DFMNavWidget * m_navWidget = nullptr;      //导航小部件
    DFMAddressBar *m_addressBar = nullptr;     //地址編輯栏
    QToolButton *m_searchButton = nullptr;     //搜索栏按钮
    QToolButton *m_searchFilterButton = nullptr; //搜索过滤按钮
    DFMOptionButtonBox * m_optionButtonBox = nullptr; //功能按鈕栏
    DFMCrumbBar *m_crumbBar = nullptr;         //面包屑

    DFMDisplayDefaultView m_displayCheckView;      //默认检查Url展示的View
    QWidget *m_displayWidget = nullptr;            //居中組件背板
    QVBoxLayout *m_displayViewLayout = nullptr;    //居中組件布局
    QHash<QString, DFMDisplayViewLogic*> m_views;  //展示的view类

    QWidget *m_propertyView = nullptr;        //右侧属性界面
    DFMSplitter *m_splitter = nullptr;        //布局


    DFMBrowseView::ViewMode m_mode = DFMBrowseView::ListMode;

public:
    explicit DFMBrowseWindowPrivate(DFMBrowseWindow *qq);

    void setDefaultViewMode(DFMBrowseView::ViewMode mode);

    void addview(const QString &scheme,
                 DFMDisplayViewLogic *logic);

    bool viewIsAdded(const QString &scheme);

    DButtonBoxButton *navBackButton() const;
    void setNavBackButton(DButtonBoxButton *navBackButton);

    DButtonBoxButton *navForwardButton() const;
    void setNavForwardButton(DButtonBoxButton *navForwardButton);

    QToolButton *searchButton() const;
    void setSearchButton(QToolButton* searchButton);

    QToolButton *searchFilterButton() const;
    void setSearchFilterButton(QToolButton *searchFilterButton);

    DFMOptionButtonBox *optionButtonBox() const;
    void setOptionButtonBox(DFMOptionButtonBox *optionButtonBox);

    QToolButton *listViewButton() const;
    void setListViewButton(QToolButton *button);

    QToolButton *iconViewButton() const;
    void setIconViewButton(QToolButton *button);

    DFMCrumbBar *crumbBar() const;
    void setCrumbBar(DFMCrumbBar *crumbBar);

    QWidget *propertyView() const;
    void setPropertyView(QWidget *propertyView);

    DFMSideBar *sidebar() const;
    void setSidebar(DFMSideBar *sidebar);

    DFMAddressBar *addressBar() const;
    void setAddressBar(DFMAddressBar *addressBar);

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

#endif // DFMBROWSEWINDOWPRIVATE_H
