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
class EnterDirAnimationWidget;
class WorkspaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    using ViewPtr = DFMBASE_NAMESPACE::AbstractBaseView *;
    using TopWidgetPtr = QSharedPointer<QWidget>;

public:
    explicit WorkspaceWidget(QFrame *parent = nullptr);

    ViewPtr currentViewPtr() const;
    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode() const;
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;

    DFMBASE_NAMESPACE::AbstractBaseView *currentView();

    void setCustomTopWidgetVisible(const QString &scheme, bool visible);
    bool getCustomTopWidgetVisible(const QString &scheme);

    QRectF viewVisibleGeometry();
    QRectF itemRect(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);

public slots:
    void onCreateNewWindow();
    void onRefreshCurrentView();
    void handleViewStateChanged();
    void handleAboutPlaySplitterAnim(int startValue, int endValue);

protected:
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

public slots:
    void initUiForSizeMode();
    void onAnimDelayTimeout();

private:
    void initializeUi();
    void initViewLayout();
    void initCustomTopWidgets(const QUrl &url);
    void setCurrentView(const QUrl &url);

    QUrl workspaceUrl;
    QVBoxLayout *widgetLayout { nullptr };
    QStackedLayout *viewStackLayout { nullptr };
    QMap<QString, ViewPtr> views;
    QMap<QString, TopWidgetPtr> topWidgets;
    EnterDirAnimationWidget *enterAnim { nullptr };
    QTimer *appearAnimDelayTimer { nullptr };

    bool canPlayAppearAnimation { false };
};

}

#endif   // WORKSPACEWIDGET_H
