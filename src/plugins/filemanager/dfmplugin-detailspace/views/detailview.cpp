// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailview.h"
#include "imagepreviewwidget.h"
#include "imagepreviewworker.h"
#include "fileinfowidget.h"
#include "utils/detailmanager.h"

#include <dfm-base/base/schemefactory.h>

#include <QScrollArea>
#include <QResizeEvent>

#include <DFrame>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_detailspace;

DetailView::DetailView(QWidget *parent)
    : DFrame(parent)
{
    initInfoUI();
}

DetailView::~DetailView()
{
}

void DetailView::setUrl(const QUrl &url)
{
    if (url == m_currentUrl)
        return;

    m_currentUrl = url;
    updateHeadUI(url);
    updateBasicWidget(url);
    updateExtensionWidgets(url);
}

void DetailView::onPreviewReady(const QUrl &url, const QPixmap &pixmap)
{
    if (url != m_currentUrl) {
        return;
    }

    finishPreviewLoading();

    if (m_previewWidget) {
        m_previewWidget->stopAnimatedImage();
        m_previewWidget->setPixmap(pixmap);
    }
}

void DetailView::onAnimatedImageReady(const QUrl &url, const QString &filePath)
{
    if (url != m_currentUrl) {
        return;
    }

    finishPreviewLoading();

    if (m_previewWidget) {
        m_previewWidget->setAnimatedImage(filePath);
    }
}

void DetailView::initInfoUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setAlignment(Qt::AlignLeft);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    expandFrame = new QFrame(this);
    expandFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(expandFrame);

    vLayout = new QVBoxLayout;
    vLayout->addStretch();
    expandFrame->setLayout(vLayout);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);

    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(scrollArea);
    setFrameShape(QFrame::NoFrame);
    setLayout(mainLayout);

    // Create preview controller
    m_previewController = new ImagePreviewController(this);
    connect(m_previewController, &ImagePreviewController::previewReady,
            this, &DetailView::onPreviewReady);
    connect(m_previewController, &ImagePreviewController::animatedImageReady,
            this, &DetailView::onAnimatedImageReady);

    // Create core widgets once - they will be reused across URL changes
    // This is the key optimization to prevent UI flicker!

    // 1. Create preview widget (index 0)
    m_previewWidget = new ImagePreviewWidget(this);
    vLayout->insertWidget(0, m_previewWidget, 0, Qt::AlignHCenter);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() {
                reloadPreviewFile();
            });

    connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::iconThemeNameChanged, this, [this]() {
        reloadPreviewFile();
    });

    // 2. Create file info widget (index 1) - with separator frame
    QFrame *infoFrame = new QFrame(this);
    auto separator = new DHorizontalLine(infoFrame);
    separator->setFixedHeight(1);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(separator);

    m_fileInfoWidget = new FileInfoWidget(this);
    m_fileInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    infoLayout->addWidget(m_fileInfoWidget);
    infoFrame->setLayout(infoLayout);
    vLayout->insertWidget(1, infoFrame, 0, Qt::AlignTop);

    // 3. Create extension widgets once
    createExtensionWidgets();

    // 4. Create spinner overlay for loading state
    initSpinnerOverlay();
}

void DetailView::initSpinnerOverlay()
{
    // Create overlay as child of preview widget (covers preview area)
    m_spinnerOverlay = new QWidget(m_previewWidget);
    m_spinnerOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    QVBoxLayout *overlayLayout = new QVBoxLayout(m_spinnerOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(0, 0, 0, 0);

    m_spinner = new DSpinner(m_spinnerOverlay);
    m_spinner->setFixedSize(32, 32);
    overlayLayout->addWidget(m_spinner, 0, Qt::AlignCenter);

    m_spinnerOverlay->hide();

    // Create loading timer (single-shot)
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setSingleShot(true);
    m_loadingTimer->setInterval(kSpinnerDelayMs);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        if (m_spinnerOverlay && m_spinner) {
            m_spinnerOverlay->show();
            m_spinner->start();
        }
    });
}

void DetailView::createExtensionWidgets()
{
    // Get all registered extensions and create widgets once
    const auto extensions = DetailManager::instance().extensionInfos();

    for (const auto &extInfo : extensions) {
        // Create widget using create function (pass empty URL, will be updated later)
        QWidget *widget = extInfo.create(QUrl());
        if (!widget)
            continue;

        widget->setParent(this);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        if (auto layout = widget->layout())
            layout->setContentsMargins(0, 0, 0, 0);

        // Wrap with separator frame
        QFrame *frame = new QFrame(this);
        auto *separator = new DHorizontalLine(frame);
        separator->setFixedHeight(1);
        QVBoxLayout *frameLayout = new QVBoxLayout(frame);
        frameLayout->setContentsMargins(0, 0, 0, 0);
        frameLayout->addWidget(separator);
        frameLayout->addWidget(widget);
        frame->setLayout(frameLayout);

        // Add to layout (after core widgets, before stretch)
        vLayout->insertWidget(vLayout->count() - 1, frame, 0, Qt::AlignTop);

        // Store holder for later updates
        ExtensionWidgetHolder holder;
        holder.widget = widget;
        holder.frame = frame;
        holder.update = extInfo.update;
        holder.shouldShow = extInfo.shouldShow;
        m_extensionWidgets.append(holder);

        // Initially hidden until setUrl is called
        frame->setVisible(false);
    }
}

void DetailView::updateHeadUI(const QUrl &url)
{
    // Preview widget is already created in initInfoUI()
    // Just update its content here
    m_previewWidget->show();
    updatePreviewSize();

    // Start loading state (clears old preview, starts delayed spinner timer)
    startPreviewLoading();

    // Always request preview at maximum size (500px width)
    // This ensures smooth resizing without reloading - paintEvent will scale it
    m_previewController->requestPreview(url, ImagePreviewWidget::maximumPreviewSize());
}

void DetailView::updateBasicWidget(const QUrl &url)
{
    // FileInfoWidget is already created in initInfoUI()
    // Just update its content here - no widget creation/deletion!
    m_fileInfoWidget->show();
    m_fileInfoWidget->setUrl(url);
}

void DetailView::updateExtensionWidgets(const QUrl &url)
{
    // Update all extension widgets - no deletion/recreation!
    for (auto &holder : m_extensionWidgets) {
        // Check if widget should be visible for this URL
        bool visible = holder.shouldShow(url);
        holder.frame->setVisible(visible);

        if (visible) {
            // Update widget data
            holder.update(holder.widget, url);
        }
    }
}

void DetailView::updatePreviewSize()
{
    if (m_previewWidget) {
        QSize hint = m_previewWidget->sizeHint();
        m_previewWidget->setFixedHeight(hint.height());
        m_previewWidget->update();

        // Sync spinner overlay size with preview widget
        if (m_spinnerOverlay) {
            m_spinnerOverlay->setFixedSize(m_previewWidget->size());
        }
    }
}

void DetailView::reloadPreviewFile()
{
    if (!m_currentUrl.isEmpty() && m_previewWidget) {
        m_previewController->requestPreview(m_currentUrl,
                                            ImagePreviewWidget::maximumPreviewSize());
    }
}

void DetailView::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updatePreviewSize();

    // All custom control widgets automatically resize with the layout system
    // thanks to QSizePolicy::Expanding. No manual width adjustment needed!

    // No need to reload preview on resize!
    // We always load images at maximum size (500px), and paintEvent scales them.
    // This approach (inspired by Dolphin) provides smooth, immediate resizing
    // without the CPU overhead of reloading large images.
}

void DetailView::startPreviewLoading()
{
    // Clear current preview to avoid showing stale content
    if (m_previewWidget) {
        m_previewWidget->stopAnimatedImage();
        m_previewWidget->setPixmap(QPixmap());
    }

    // Start delayed timer - spinner only shows if loading takes longer than threshold
    if (m_loadingTimer) {
        m_loadingTimer->start();
    }
}

void DetailView::finishPreviewLoading()
{
    // Stop the timer (prevents spinner from showing if load was fast)
    if (m_loadingTimer) {
        m_loadingTimer->stop();
    }

    // Hide spinner if it was shown
    if (m_spinner) {
        m_spinner->stop();
    }
    if (m_spinnerOverlay) {
        m_spinnerOverlay->hide();
    }
}
