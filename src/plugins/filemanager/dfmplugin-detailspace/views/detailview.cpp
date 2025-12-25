// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

bool DetailView::addCustomWidget(QWidget *widget)
{
    if (widget) {
        // -1 means append at the end (before stretch)
        insertCustomWidget(-1, widget);
        return true;
    }
    return false;
}

bool DetailView::insertCustomWidget(int index, QWidget *widget)
{
    // Core widgets occupy indices 0 (preview) and 1 (file info)
    // Extension widgets start from index 2
    // The stretch item is at the end of the layout
    static constexpr int kCoreWidgetCount = 2;

    // Calculate actual insert position
    // -1 means append before stretch, otherwise offset by core widget count
    int actualIndex = (index == -1)
            ? vLayout->count() - 1   // Before stretch
            : qMin(vLayout->count() - 1, index + kCoreWidgetCount);

    if (widget) {
        widget->setParent(this);
        // Don't set MaximumWidth - let widget follow parent container width naturally
        // Widget will resize automatically with the layout system
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QFrame *frame = new QFrame(this);
        auto *seperator = new DHorizontalLine(frame);
        seperator->setFixedHeight(1);
        QVBoxLayout *vlayout = new QVBoxLayout(frame);
        vlayout->setContentsMargins(0, 0, 0, 0);
        vlayout->addWidget(seperator);
        vlayout->addWidget(widget);
        frame->setLayout(vlayout);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->insertWidget(actualIndex, frame, 0, Qt::AlignTop);

        expandList.append(frame);
        return true;
    }
    return false;
}

void DetailView::removeWidget()
{
    for (QWidget *w : expandList) {
        expandList.removeOne(w);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->removeWidget(w);
        delete w;
    }
    // Note: m_fileInfoWidget is NOT deleted here - it's reused across URL changes
    // This prevents UI flicker and improves performance
}

void DetailView::setUrl(const QUrl &url)
{
    m_currentUrl = url;
    updateHeadUI(url);
    updateBasicWidget(url);
    createExtensionWidgets(url);
}

void DetailView::onPreviewReady(const QUrl &url, const QPixmap &pixmap)
{
    if (url != m_currentUrl) {
        return;
    }

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
                if (!m_currentUrl.isEmpty() && m_previewWidget) {
                    m_previewController->requestPreview(m_currentUrl,
                                                        ImagePreviewWidget::maximumPreviewSize());
                }
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
    // Note: infoFrame is NOT added to expandList - it's a core widget!
}

void DetailView::updateHeadUI(const QUrl &url)
{
    // Preview widget is already created in initInfoUI()
    // Just update its content here
    m_previewWidget->show();
    updatePreviewSize();

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

void DetailView::updatePreviewSize()
{
    if (m_previewWidget) {
        QSize hint = m_previewWidget->sizeHint();
        m_previewWidget->setFixedHeight(hint.height());
        m_previewWidget->update();
    }
}

void DetailView::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updatePreviewSize();

    // All custom control widgets (including m_fileBaseInfoView) automatically
    // resize with the layout system thanks to QSizePolicy::Expanding.
    // No manual width adjustment needed!

    // No need to reload preview on resize!
    // We always load images at maximum size (500px), and paintEvent scales them.
    // This approach (inspired by Dolphin) provides smooth, immediate resizing
    // without the CPU overhead of reloading large images.
}

void DetailView::createExtensionWidgets(const QUrl &url)
{
    // Create extension widgets provided by other plugins
    // This centralizes widget lifecycle management in DetailView::setUrl()
    // following Single Responsibility Principle
    QMap<int, QWidget *> widgetMap = DetailManager::instance().createExtensionView(url);
    if (!widgetMap.isEmpty()) {
        QList<int> indexes = widgetMap.keys();
        for (int index : indexes) {
            insertCustomWidget(index, widgetMap.value(index));
        }
    }
}
