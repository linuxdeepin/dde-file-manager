// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/dfm_global_defines.h>

#include "dtkwidget_global.h"

#include <QUrl>
#include <QMap>
#include <DFrame>

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

namespace dfmplugin_workspace {

class TabBar;
class FileView;
class WorkspaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    using ViewPtr = QSharedPointer<DFMBASE_NAMESPACE::AbstractBaseView>;
    using TopWidgetPtr = QSharedPointer<QWidget>;

public:
    explicit WorkspaceWidget(QFrame *parent = nullptr);

    ViewPtr currentViewPtr() const;
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode() const;
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;

    DFMBASE_NAMESPACE::AbstractBaseView *currentView();

    void openNewTab(const QUrl &url);
    bool canAddNewTab();
    void closeTab(quint64 winId, const QUrl &url);
    void setTabAlias(const QUrl &url, const QString &newName);
    void setCustomTopWidgetVisible(const QString &scheme, bool visible);
    bool getCustomTopWidgetVisible(const QString &scheme);

    QRectF viewVisibleGeometry();
    QRectF itemRect(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);

public slots:
    void onOpenUrlInNewTab(quint64 windowId, const QUrl &url);
    void onCurrentTabChanged(int tabIndex);
    void onRequestCloseTab(const int index, const bool &remainState);
    void onTabAddableChanged(bool addable);
    void showNewTabButton();
    void hideNewTabButton();
    void onNewTabButtonClicked();
    void onNextTab();
    void onPreviousTab();
    void onCloseCurrentTab();
    void onCreateNewTab();
    void onCreateNewWindow();
    void onSetCurrentTabIndex(const int index);
    void onRefreshCurrentView();

protected:
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

public slots:
    void initUiForSizeMode();

private:
    void initializeUi();
    void initConnect();
    void initTabBar();
    void initViewLayout();
    void initCustomTopWidgets(const QUrl &url);
    void setCurrentView(const QUrl &url);

    QUrl workspaceUrl;
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

}

#endif   // WORKSPACEWIDGET_H
