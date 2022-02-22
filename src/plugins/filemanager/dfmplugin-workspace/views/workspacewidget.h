/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractframe.h"

#include "dtkwidget_global.h"

#include <QUrl>
#include <QMap>

namespace DFMBASE_NAMESPACE {
class AbstractBaseView;
}   // namespace dfmbase

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DHorizontalLine;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QVBoxLayout;
class QStackedLayout;
QT_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class TabBar;
class FileView;
class WorkspaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    using ViewPtr = QSharedPointer<DFMBASE_NAMESPACE::AbstractBaseView>;
    using TopWidgetPtr = QSharedPointer<QWidget>;

public:
    explicit WorkspaceWidget(QFrame *parent = nullptr);

    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;

    void openNewTab(const QUrl &url);
    bool canAddNewTab();
    void closeTab(quint64 winId, const QString &path);
    void setCustomTopWidgetVisible(const QString &scheme, bool visible);
    bool getCustomTopWidgetVisible(const QString &scheme);

public slots:
    void onOpenUrlInNewTab(quint64 windowId, const QUrl &url);
    void onCurrentTabChanged(int tabIndex);
    void onRequestCloseTab(const int index, const bool &remainState);
    void onTabAddableChanged(bool addable);
    void showNewTabButton();
    void hideNewTabButton();
    void onNewTabButtonClicked();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initializeUi();
    void initConnect();
    void initTabBar();
    void initViewLayout();
    void handleCtrlN();
    void initCustomTopWidgets(const QUrl &url);

    QUrl workspaceUrl;
    QFrame *topWidgetContainer { nullptr };
    QHBoxLayout *tabBarLayout { nullptr };
    QVBoxLayout *widgetLayout { nullptr };
    QStackedLayout *viewStackLayout { nullptr };
    QMap<QString, ViewPtr> views;
    QMap<QString, TopWidgetPtr> topWidgets;

    TabBar *tabBar { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *newTabButton { nullptr };
    DTK_WIDGET_NAMESPACE::DHorizontalLine *tabTopLine { nullptr };
    DTK_WIDGET_NAMESPACE::DHorizontalLine *tabBottomLine { nullptr };
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEWIDGET_H
