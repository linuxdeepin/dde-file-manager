// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    TabBar *tabBar() const;
    CrumbBar *titleCrumbBar() const;
    void openNewTab(const QUrl &url);
    void openCustomFixedTabs();
    void openPinnedTabs();
    void activatePinnedTab(const QString &pinnedId);

    void showSearchFilterButton(bool visible);
    void setViewModeState(int mode);
    void handleSplitterAnimation(const QVariant &position);

    int calculateRemainingWidth() const;

public slots:
    void handleHotkeyCtrlF();
    void handleHotkeyCtrlL();
    void handleHotketSwitchViewMode(int mode);
    void handleHotketNextTab();
    void handleHotketPreviousTab();
    void handleHotketCloseCurrentTab();
    void handleHotketCreateNewTab();
    void handleCreateTabList(const QList<QUrl> &urlList);
    void handleHotketActivateTab(const int index);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void updateUiForSizeMode();
    void showAddrsssBar(const QUrl &url);   // switch addrasssBar and crumbBar show
    void showCrumbBar();
    bool eventFilter(QObject *watched, QEvent *event) override;

    void saveTitleBarState(const QString &uniqueId);
    void restoreTitleBarState(const QString &uniqueId);
    bool checkCustomFixedTab(int index);

signals:
    void currentUrlChanged(const QUrl &url);

private slots:
    void onAddressBarJump();
    void onTabCreated();
    void handleCreateView(const QString &uniqueId);
    void onTabAboutToRemove(int oldIndex, int nextIndex);
    void onTabMoved(int from, int to);
    void onTabCurrentChanged(int oldIndex, int newIndex);
    void onTabCloseRequested(int index);
    void onTabAddButtonClicked();
    void quitSearch();

private:
    QUrl titlebarUrl;
    QString pendingPinnedTabId;  // Store pinnedId from pinned:// URL
    DTitlebar *topBar { nullptr };
    TabBar *bottomBar { nullptr };
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

    struct TitleBarState
    {
        DFMBASE_NAMESPACE::Global::ViewMode viewMode { DFMBASE_NAMESPACE::Global::ViewMode::kIconMode };
        bool advancedSearchVisible { false };
        bool advancedSearchChecked { false };
        QString searchText { "" };
    };

    QMap<QString, TitleBarState> titleBarStateMap;
};

}

#endif   // TITLEBARWIDGET_H
