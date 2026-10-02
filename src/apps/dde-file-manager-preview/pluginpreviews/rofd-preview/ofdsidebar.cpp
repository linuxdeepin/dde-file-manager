// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ofdsidebar.h"

#include <dfm-base/utils/rofd/rofdrenderer.h>

#include <DGuiApplicationHelper>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPainter>
#include <QPainterPath>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE

namespace plugin_filepreview {

namespace {
constexpr int kSidebarWidth = 150;
constexpr int kThumbWidth = 128;
constexpr int kThumbRadius = 10;   // at 2x thumbnail resolution (~5px on screen)
}

OfdSidebar::OfdSidebar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    list = new QListWidget(this);
    list->setWrapping(false);
    list->setIconSize(QSize(kThumbWidth, kThumbWidth));
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setFrameShape(QFrame::NoFrame);
    // jump-on-click without painting the selection activation color
    list->setSelectionMode(QAbstractItemView::NoSelection);
    QPalette pal = list->palette();
    pal.setColor(QPalette::Highlight, Qt::transparent);
    list->setPalette(pal);
    layout->addWidget(list);

    setFixedWidth(kSidebarWidth);

    connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        Q_EMIT pageClicked(list->row(item));
    });

    // re-compose rendered thumbnails so the border follows theme switches
    // live (same signal the pdf preview uses)
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
            this, [this]() {
        for (int idx : pageThumbs.keys()) {
            applyItemIcon(idx);
        }
    });
}

OfdSidebar::~OfdSidebar()
{
    if (cancelFlag) {
        cancelFlag->store(true);
    }
}

void OfdSidebar::init(int pageCount, const QString &path)
{
    // cancel any thumbnail job left over from a previous document
    if (cancelFlag) {
        cancelFlag->store(true);
    }
    cancelFlag.reset(new std::atomic_bool(false));
    filePath = path;
    pages = pageCount;
    currentPageIdx = 0;
    pageThumbs.clear();

    list->clear();
    for (int i = 0; i < pages; ++i) {
        auto *item = new QListWidgetItem(list);
        item->setText(QString::number(i + 1));
        item->setTextAlignment(Qt::AlignHCenter);
    }

    startThumbnailJob();
}

void OfdSidebar::applyItemIcon(int index)
{
    if (index < 0 || index >= list->count() || !pageThumbs.contains(index)) {
        return;
    }
    const QImage &img = pageThumbs.value(index);
    QPixmap pm(img.size());
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    // clip the page image to a rounded rect so no square corner sticks out
    QPainterPath clip;
    clip.addRoundedRect(QRectF(0, 0, img.width(), img.height()),
                        kThumbRadius, kThumbRadius);
    p.setClipPath(clip);
    p.drawImage(0, 0, img);
    p.setClipping(false);

    if (index == currentPageIdx) {
        // thin rounded border composed into the pixmap itself, so it hugs
        // the thumbnail edge at any scale; 2x-resolution pen ≈ 2px on screen.
        // highlight() follows the system theme, same as the pdf preview
        QColor accent = Dtk::Gui::DGuiApplicationHelper::instance()
                                ->applicationPalette().highlight().color();
        if (!accent.isValid() || accent.alpha() == 0) {
            accent = QColor(0, 129, 255);
        }
        QPen pen(accent, qMax(2, 2 * img.width() / kThumbWidth));
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(1, 1, img.width() - 2.0, img.height() - 2.0),
                          kThumbRadius - 1, kThumbRadius - 1);
    }
    p.end();
    list->item(index)->setIcon(QIcon(pm));
}

void OfdSidebar::setCurrentPage(int index)
{
    if (index < 0 || index >= list->count()) {
        return;
    }
    const int old = currentPageIdx;
    currentPageIdx = index;
    applyItemIcon(old);
    applyItemIcon(index);
    list->scrollToItem(list->item(index), QAbstractItemView::EnsureVisible);
}

void OfdSidebar::startThumbnailJob()
{
    if (pages < 1 || filePath.isEmpty()) {
        return;
    }

    ++thumbGeneration;
    const int generation = thumbGeneration;
    auto cancelled = cancelFlag;
    const QString path = filePath;
    const int total = pages;
    const int dpr = qMax(1, qRound(devicePixelRatioF()));
    const int renderWidth = kThumbWidth * 2 * dpr;

    // One background job walks all pages sequentially; each finished page is
    // posted back to the GUI thread. `cancelled` is shared with the object so
    // destruction or re-init stops the loop safely.
    QtConcurrent::run([this, cancelled, generation, path, total, renderWidth]() {
        RofdRenderer renderer;
        if (!renderer.openDocument(path)) {
            return;
        }
        for (int i = 0; i < total; ++i) {
            if (cancelled->load()) {
                return;
            }
            const QImage image = renderer.renderPage(i, renderWidth);
            if (cancelled->load() || image.isNull()) {
                continue;
            }
            QMetaObject::invokeMethod(this, [this, cancelled, generation, i, image]() {
                if (cancelled->load() || generation != thumbGeneration || i >= list->count()) {
                    return;
                }
                pageThumbs.insert(i, image);
                applyItemIcon(i);
            }, Qt::QueuedConnection);
        }
    });
}
}
