/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include "dfmplugin_titlebar_global.h"
#include "views/navwidget.h"
#include "views/addressbar.h"
#include "views/crumbbar.h"
#include "views/optionbuttonbox.h"

#include "dfm-base/interfaces/abstractframe.h"

#include <QHBoxLayout>
#include <QToolButton>

DPTITLEBAR_BEGIN_NAMESPACE

class TitleBarWidget : public dfmbase::AbstractFrame
{
    Q_OBJECT
public:
    explicit TitleBarWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;
    NavWidget *navWidget() const;

public slots:
    void handleHotkeyCtrlF();
    void handleHotkeyCtrlL();

private:
    void initializeUi();
    void initConnect();
    void showAddrsssBar(const QUrl &url);   // switch addrasssBar and crumbBar show
    void showAddressBar(const QString &text);
    void showCrumbBar();
    void showSearchButton();
    void showSearchFilterButton();
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void currentUrlChanged(const QUrl &url);

private slots:
    void onSearchButtonClicked();
    void onAddressBarJump();

private:
    QUrl titlebarUrl;
    QHBoxLayout *titleBarLayout { nullptr };   // 标题栏布局
    NavWidget *curNavWidget { nullptr };   // 导航小部件
    AddressBar *addressBar { nullptr };   // 地址编辑栏
    QToolButton *searchButton { nullptr };   // 搜索栏按钮
    QToolButton *searchFilterButton { nullptr };   // 搜索过滤按钮
    OptionButtonBox *optionButtonBox { nullptr };   // 功能按鈕栏
    CrumbBar *crumbBar { nullptr };   // 面包屑
};

DPTITLEBAR_END_NAMESPACE

#endif   // TITLEBARWIDGET_H
