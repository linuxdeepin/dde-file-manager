// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROFDPREVIEW_H
#define ROFDPREVIEW_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QPointer>
#include <QUrl>

class QFutureWatcherBase;
class QLabel;
class QTimer;
class QToolButton;

namespace plugin_filepreview {
class OfdPageView;
class OfdSidebar;

class RofdPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit RofdPreview(QObject *parent = nullptr);
    ~RofdPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QWidget *statusBarWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

    void handleBeforDestroy() override;

private Q_SLOTS:
    void onRenderFinished();
    void onViewportResized();
    void renderCurrentPage();

private:
    void switchPage(int delta);
    void jumpToPage(int index);
    void updateNavBar();

    QUrl selectFileUrl;
    QString pageTitle;

    QWidget *contentContainer = nullptr;
    OfdPageView *pageView = nullptr;
    OfdSidebar *sidebar = nullptr;

    QWidget *statusBarFrame = nullptr;
    QToolButton *prevButton = nullptr;
    QToolButton *nextButton = nullptr;
    QLabel *pageLabel = nullptr;

    int pageCount = 0;
    int currentPage = 0;
    int renderGeneration = 0;
    int renderedWidth = 0;
    int renderedPageIndex = -1;
    QFutureWatcherBase *watcher = nullptr;
    QTimer *resizeDebounce = nullptr;
};
}
#endif   // ROFDPREVIEW_H
