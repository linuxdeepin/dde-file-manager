// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include "dfmplugin_titlebar_global.h"
#include "views/navwidget.h"
#include "views/addressbar.h"
#include "views/crumbbar.h"
#include "views/optionbuttonbox.h"

#include <dfm-base/interfaces/abstractframe.h>

#include <QHBoxLayout>

DWIDGET_BEGIN_NAMESPACE
class DTitlebar;
class DIconButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

class TabBar;
class SearchEditWidget;
class TitleBarWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
public:
    explicit TitleBarWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;
    NavWidget *navWidget() const;
    DTitlebar *titleBar() const;

    void startSpinner();
    void stopSpinner();
    void showSearchFilterButton(bool visible);

    void setViewModeState(int mode);

    void initTabBar(const quint64 windowId);
    void currentTabChanged(const int index);
    void handleSplitterAnimation(const QVariant &position);

public slots:
    void handleHotkeyCtrlF();
    void handleHotkeyCtrlL();
    void handleHotketSwitchViewMode(int mode);
    void handleHotketNextTab();
    void handleHotketPreviousTab();
    void handleHotketCloseCurrentTab();
    void handleHotketCreateNewTab();
    void handleHotketActivateTab(const int index);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void initUiForSizeMode();
    void showAddrsssBar(const QUrl &url);   // switch addrasssBar and crumbBar show
    void showCrumbBar();
    bool eventFilter(QObject *watched, QEvent *event) override;
    TabBar *createTabBar(const quint64 windowId);

signals:
    void currentUrlChanged(const QUrl &url);

private slots:
    void onAddressBarJump();

    void onTabCreated();
    void onTabRemoved(int index);
    void onTabMoved(int from, int to);

private:
    QUrl titlebarUrl;
    DTitlebar *topBar { nullptr };
    QHBoxLayout *topBarCustomLayout { nullptr };
    QVBoxLayout *titleBarLayout { nullptr };   // 标题栏布局
    QHBoxLayout *bottomBarLayout { nullptr };
    NavWidget *curNavWidget { nullptr };   // 导航小部件
    AddressBar *addressBar { nullptr };   // 地址编辑栏
    SearchEditWidget *searchEditWidget { nullptr };   // 搜索编辑栏
    OptionButtonBox *optionButtonBox { nullptr };   // 功能按鈕栏
    CrumbBar *crumbBar { nullptr };   // 面包屑
    QWidget *placeholder { nullptr };
    
    bool searchButtonSwitchState { false };
};

}

#endif   // TITLEBARWIDGET_H
