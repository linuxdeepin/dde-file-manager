// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailview.h"
#include "imagepreviewwidget.h"
#include "imagepreviewworker.h"
#include "filebaseinfoview.h"

#include <dfm-base/base/schemefactory.h>

#include <QScrollArea>
#include <QResizeEvent>

#include <DPushButton>
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

bool DetailView::addCustomControl(QWidget *widget)
{
    if (widget) {
        QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
        insertCustomControl(vlayout->count() - 1, widget);
        return true;
    }
    return false;
}

bool DetailView::insertCustomControl(int index, QWidget *widget)
{
    index = index == -1 ? vLayout->count() - 1 : qMin(vLayout->count() - 1, index);

    if (widget) {
        widget->setParent(this);
        widget->setMaximumWidth(scrollArea->width() - vLayout->contentsMargins().right());
        QFrame *frame = new QFrame(this);
        DPushButton *btn = new DPushButton(frame);
        btn->setEnabled(false);
        btn->setFixedHeight(1);
        QVBoxLayout *vlayout = new QVBoxLayout(frame);
        vlayout->setContentsMargins(0, 0, 0, 0);
        vlayout->setSpacing(10);
        vlayout->addWidget(btn);
        vlayout->addWidget(widget);
        frame->setLayout(vlayout);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->insertWidget(index, frame, 0, Qt::AlignTop);

        expandList.append(frame);
        return true;
    }
    return false;
}

void DetailView::removeControl()
{
    for (QWidget *w : expandList) {
        expandList.removeOne(w);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->removeWidget(w);
        delete w;
    }
    // Reset file base info view pointer since it was deleted
    m_fileBaseInfoView = nullptr;
}

void DetailView::setUrl(const QUrl &url, int widgetFilter)
{
    m_currentUrl = url;
    createHeadUI(url, widgetFilter);
    createBasicWidget(url, widgetFilter);
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
    vLayout->setContentsMargins(0, 0, 8, 0);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea, Qt::AlignCenter);
    setFrameShape(QFrame::NoFrame);
    setLayout(mainLayout);

    // Create preview controller
    m_previewController = new ImagePreviewController(this);
    connect(m_previewController, &ImagePreviewController::previewReady,
            this, &DetailView::onPreviewReady);
    connect(m_previewController, &ImagePreviewController::animatedImageReady,
            this, &DetailView::onAnimatedImageReady);
}

void DetailView::createHeadUI(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DetailFilterType::kIconView) {
        if (m_previewWidget) {
            m_previewWidget->hide();
        }
        return;
    }

    // Reuse preview widget instead of recreating
    if (!m_previewWidget) {
        m_previewWidget = new ImagePreviewWidget(this);
        vLayout->insertWidget(0, m_previewWidget, 0, Qt::AlignHCenter);

        connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                this, [this]() {
                    if (!m_currentUrl.isEmpty() && m_previewWidget) {
                        // Always request preview at maximum size for best quality
                        m_previewController->requestPreview(m_currentUrl,
                                                           ImagePreviewWidget::maximumPreviewSize());
                    }
                });
    }

    m_previewWidget->show();
    updatePreviewSize();

    // Always request preview at maximum size (500px width)
    // This ensures smooth resizing without reloading - paintEvent will scale it
    m_previewController->requestPreview(url, ImagePreviewWidget::maximumPreviewSize());
}

void DetailView::createBasicWidget(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DetailFilterType::kBasicView) {
        if (m_fileBaseInfoView) {
            m_fileBaseInfoView->hide();
        }
        return;
    }

    // Reuse FileBaseInfoView instead of recreating
    if (!m_fileBaseInfoView) {
        m_fileBaseInfoView = new FileBaseInfoView(this);
        addCustomControl(m_fileBaseInfoView);
    }

    m_fileBaseInfoView->show();
    m_fileBaseInfoView->setFileUrl(url);
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

    // Update info view width
    if (m_fileBaseInfoView) {
        m_fileBaseInfoView->setMaximumWidth(event->size().width() - 20);
    }

    // No need to reload preview on resize!
    // We always load images at maximum size (500px), and paintEvent scales them.
    // This approach (inspired by Dolphin) provides smooth, immediate resizing
    // without the CPU overhead of reloading large images.
}
