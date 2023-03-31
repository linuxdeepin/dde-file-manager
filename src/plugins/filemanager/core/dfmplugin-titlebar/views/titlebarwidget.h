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

namespace dfmplugin_titlebar {

class TitleBarWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
public:
    explicit TitleBarWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;
    NavWidget *navWidget() const;

    void startSpinner();
    void stopSpinner();
    void showSearchFilterButton(bool visible);

    void setViewModeState(int mode);

public slots:
    void handleHotkeyCtrlF();
    void handleHotkeyCtrlL();
    void handleHotketSwitchViewMode(int mode);

private:
    void initializeUi();
    void initConnect();
    void initUiForSizeMode();
    void showAddrsssBar(const QUrl &url);   // switch addrasssBar and crumbBar show
    void showCrumbBar();
    void showSearchButton();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void toggleSearchButtonState(bool switchBtn = true);

signals:
    void currentUrlChanged(const QUrl &url);

private slots:
    void onSearchButtonClicked();
    void onAddressBarJump();
    void searchBarActivated();
    void searchBarDeactivated();

private:
    QUrl titlebarUrl;
    QHBoxLayout *titleBarLayout { nullptr };   // 标题栏布局
    NavWidget *curNavWidget { nullptr };   // 导航小部件
    AddressBar *addressBar { nullptr };   // 地址编辑栏
    DToolButton *searchButton { nullptr };   // 搜索栏按钮
    OptionButtonBox *optionButtonBox { nullptr };   // 功能按鈕栏
    CrumbBar *crumbBar { nullptr };   // 面包屑
    bool searchButtonSwitchState { false };
};

}

#endif   // TITLEBARWIDGET_H
