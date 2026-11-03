// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rofdpreview.h"
#include "ofdpageview.h"
#include "ofdsidebar.h"

#include <dfm-base/utils/rofd/rofdrenderer.h>

#include <QFileInfo>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

namespace {
static QImage renderPageJob(const QString &filePath, int pageIndex, int width)
{
    // Each job owns its own document/renderer handles, so jobs are safe to
    // run concurrently and a stale job never touches live state.
    RofdRenderer renderer;
    if (!renderer.openDocument(filePath)) {
        return QImage();
    }
    return renderer.renderPage(pageIndex, width);
}
}

RofdPreview::RofdPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "OFD preview: RofdPreview instance created";

    auto *futureWatcher = new QFutureWatcher<QImage>(this);
    connect(futureWatcher, &QFutureWatcher<QImage>::finished, this, &RofdPreview::onRenderFinished);
    watcher = futureWatcher;

    resizeDebounce = new QTimer(this);
    resizeDebounce->setSingleShot(true);
    resizeDebounce->setInterval(150);
    connect(resizeDebounce, &QTimer::timeout, this, &RofdPreview::renderCurrentPage);
}

RofdPreview::~RofdPreview()
{
    fmInfo() << "OFD preview: RofdPreview instance destroyed";
    renderGeneration++;
    watcher->disconnect();
    if (contentContainer) {
        // the container parents both the sidebar and the page view
        contentContainer->deleteLater();
    }
    if (statusBarFrame) {
        statusBarFrame->deleteLater();
    }
}

bool RofdPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "OFD preview: setting file URL:" << url;

    if (selectFileUrl == url) {
        fmDebug() << "OFD preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.isLocalFile()) {
        fmWarning() << "OFD preview: URL is not a local file:" << url;
        return false;
    }

    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        fmWarning() << "OFD preview: file does not exist:" << filePath;
        return false;
    }

    RofdRenderer renderer;
    if (!renderer.openDocument(filePath)) {
        fmWarning() << "OFD preview: cannot open OFD file:" << filePath;
        return false;
    }
    const int count = renderer.pageCount();
    if (count < 1) {
        fmWarning() << "OFD preview: file has no pages:" << filePath;
        return false;
    }

    if (contentContainer == nullptr) {
        contentContainer = new QWidget();
        auto *layout = new QHBoxLayout(contentContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        sidebar = new OfdSidebar(contentContainer);
        sidebar->setVisible(false);   // shown only for multi-page documents
        layout->addWidget(sidebar);

        pageView = new OfdPageView();
        connect(pageView, &OfdPageView::viewportResized, this, &RofdPreview::onViewportResized);
        layout->addWidget(pageView, 1);

        connect(sidebar, &OfdSidebar::pageClicked, this, &RofdPreview::jumpToPage);
    }

    if (statusBarFrame == nullptr) {
        statusBarFrame = new QWidget();
        auto *layout = new QHBoxLayout(statusBarFrame);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        prevButton = new QToolButton(statusBarFrame);
        prevButton->setIcon(statusBarFrame->style()->standardIcon(QStyle::SP_ArrowBack));
        prevButton->setAutoRaise(true);
        nextButton = new QToolButton(statusBarFrame);
        nextButton->setIcon(statusBarFrame->style()->standardIcon(QStyle::SP_ArrowForward));
        nextButton->setAutoRaise(true);
        pageLabel = new QLabel(statusBarFrame);

        layout->addWidget(prevButton);
        layout->addWidget(pageLabel);
        layout->addWidget(nextButton);

        connect(prevButton, &QToolButton::clicked, this, [this]() { switchPage(-1); });
        connect(nextButton, &QToolButton::clicked, this, [this]() { switchPage(1); });
    }

    selectFileUrl = url;
    pageTitle = QFileInfo(filePath).fileName();
    pageCount = count;
    currentPage = 0;
    renderedWidth = 0;
    renderedPageIndex = -1;

    if (count >= 2) {
        sidebar->init(count, filePath);
        sidebar->setVisible(true);
    } else {
        sidebar->setVisible(false);
    }
    updateNavBar();

    fmInfo() << "OFD preview: file URL set successfully:" << url << "title:" << pageTitle << "pages:" << pageCount;

    Q_EMIT titleChanged();

    renderCurrentPage();

    return true;
}

QUrl RofdPreview::fileUrl() const
{
    return selectFileUrl;
}

QWidget *RofdPreview::contentWidget() const
{
    // the container holds the thumbnail sidebar and the page view
    return contentContainer ? contentContainer : pageView;
}

QWidget *RofdPreview::statusBarWidget() const
{
    return statusBarFrame;
}

QString RofdPreview::title() const
{
    return pageTitle;
}

bool RofdPreview::showStatusBarSeparator() const
{
    return false;
}

void RofdPreview::handleBeforDestroy()
{
    renderGeneration++;
    watcher->disconnect();
}

void RofdPreview::onRenderFinished()
{
    auto *futureWatcher = static_cast<QFutureWatcher<QImage> *>(watcher);
    const QImage image = futureWatcher->result();

    // Drop results of jobs scheduled before the latest page switch.
    if (futureWatcher->property("generation").toInt() != renderGeneration) {
        return;
    }

    if (pageView && !image.isNull()) {
        pageView->setPage(image);
    } else if (pageView) {
        pageView->setBusy(false);
    }
}

void RofdPreview::onViewportResized()
{
    resizeDebounce->start();
}

void RofdPreview::renderCurrentPage()
{
    if (pageView == nullptr || !selectFileUrl.isLocalFile() || pageCount < 1) {
        return;
    }

    const int dpr = qMax(1, qRound(pageView->devicePixelRatioF()));
    const int targetWidth = qMax(600, pageView->width()) * qMax(1, dpr);
    // Skip re-rendering only when it is the same page and the width barely
    // changed (resize debounce); a page switch must always re-render.
    const bool pageChanged = renderedPageIndex != currentPage;
    if (!pageChanged && renderedWidth > 0 && qAbs(renderedWidth - targetWidth) < 80) {
        return;
    }
    renderedWidth = targetWidth;
    renderedPageIndex = currentPage;

    renderGeneration++;
    auto *futureWatcher = static_cast<QFutureWatcher<QImage> *>(watcher);
    futureWatcher->setFuture(QtConcurrent::run(renderPageJob, selectFileUrl.toLocalFile(),
                                               currentPage, targetWidth));
    futureWatcher->setProperty("generation", renderGeneration);
    pageView->setBusy(true);
}

void RofdPreview::switchPage(int delta)
{
    const int target = currentPage + delta;
    if (target < 0 || target >= pageCount) {
        return;
    }
    currentPage = target;
    updateNavBar();
    if (sidebar && sidebar->isVisible()) {
        sidebar->setCurrentPage(currentPage);
    }
    renderCurrentPage();
}

void RofdPreview::jumpToPage(int index)
{
    if (index < 0 || index >= pageCount || index == currentPage) {
        return;
    }
    currentPage = index;
    updateNavBar();
    if (sidebar && sidebar->isVisible()) {
        sidebar->setCurrentPage(currentPage);
    }
    renderCurrentPage();
}

void RofdPreview::updateNavBar()
{
    if (pageLabel == nullptr) {
        return;
    }
    pageLabel->setText(QString("%1 / %2").arg(currentPage + 1).arg(pageCount));
    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage < pageCount - 1);
}
