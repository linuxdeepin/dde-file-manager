// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEPAGE_H
#define WORKSPACEPAGE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractframe.h>
#include <dfm-base/dfm_global_defines.h>

#include <QWidget>

namespace DFMBASE_NAMESPACE {
class AbstractBaseView;
}   // namespace dfmbase

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QVBoxLayout;
class QStackedLayout;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class FileView;
class EnterDirAnimationWidget;
class WorkspacePage : public QWidget
{
    Q_OBJECT

    using ViewPtr = DFMBASE_NAMESPACE::AbstractBaseView *;
    using TopWidgetPtr = QSharedPointer<QWidget>;

public:
    explicit WorkspacePage(QWidget *parent = nullptr);
    ~WorkspacePage() override;

    void setUrl(const QUrl &url);
    QUrl currentUrl() const;
    ViewPtr currentViewPtr();
    void viewStateChanged();

    void setCustomTopWidgetVisible(const QString &scheme, bool visible);
    bool getCustomTopWidgetVisible(const QString &scheme);

public Q_SLOTS:
    void onAnimDelayTimeout();

private:
    void initUI();
    void initCustomTopWidgets(const QUrl &url);
    void setCurrentView(const QUrl &url);
    void playDisappearAnimation(ViewPtr view);

    QWidget *topContainer { nullptr };   // 顶部容器
    QVBoxLayout *topLayout { nullptr };   // 顶部布局
    QWidget *viewContainer { nullptr };   // 视图容器

    QVBoxLayout *widgetLayout { nullptr };
    QStackedLayout *viewStackLayout { nullptr };

    EnterDirAnimationWidget *enterAnim { nullptr };
    QTimer *appearAnimDelayTimer { nullptr };
    bool canPlayAppearAnimation { false };

    QUrl currentPageUrl {};
    QString currentViewScheme {};

    QHash<QString, ViewPtr> views {};
    QHash<QString, TopWidgetPtr> topWidgets {};
    int highPriorityTopWidgetsCount { 0 };
};

}   // namespace dfmplugin_workspace

#endif   // WORKSPACEPAGE_H
