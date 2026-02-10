// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

class WorkspacePage;
class WorkspaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
public:
    explicit WorkspaceWidget(QFrame *parent = nullptr);

    DFMBASE_NAMESPACE::Global::ViewMode currentViewMode() const;
    void setCurrentUrl(const QUrl &url) override;
    QUrl currentUrl() const override;

    DFMBASE_NAMESPACE::AbstractBaseView *currentView() const;

    void setCustomTopWidgetVisible(const QString &scheme, bool visible);
    bool getCustomTopWidgetVisible(const QString &scheme);

    QRectF viewVisibleGeometry();
    QRectF itemRect(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);

    void createNewPage(const QString &uniqueId);
    void removePage(const QString &removedId, const QString &nextId);
    void setCurrentPage(const QString &uniqueId);

public slots:
    void onCreateNewWindow();
    void onRefreshCurrentView();
    void handleViewStateChanged();
    void handleAboutToPlaySplitterAnim(int startValue, int endValue);

protected:
    void showEvent(QShowEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    void initializeUi();
    void initViewLayout();

    QHBoxLayout *widgetLayout { nullptr };
    QStackedLayout *viewStackLayout { nullptr };

    QMap<QString, WorkspacePage *> pages;
    QString currentPageId;
};

}

#endif   // WORKSPACEWIDGET_H
