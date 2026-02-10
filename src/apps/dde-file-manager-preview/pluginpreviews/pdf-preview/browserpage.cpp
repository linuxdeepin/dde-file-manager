// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "browserpage.h"
#include "model.h"
#include "pagerenderthread.h"
#include "sheetbrowser.h"
#include "global.h"
#include "sheetrenderer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QTime>
#include <QMutexLocker>
#include <QTimer>
#include <QUuid>
#include <QDesktopServices>
#include <QDebug>
#include <QMutexLocker>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
BrowserPage::BrowserPage(SheetBrowser *parent, int index, DocSheet *sheet)
    : QGraphicsItem(), docSheet(sheet), sheetBrowserParent(parent), currentIndex(index)
{
    setAcceptHoverEvents(true);

    setFlag(QGraphicsItem::ItemIsPanel);

    originSizeF = sheet->renderer()->getPageSize(index);
}

BrowserPage::~BrowserPage()
{
    PageRenderThread::clearImageTasks(docSheet, this);
}

QRectF BrowserPage::boundingRect() const
{
    return QRectF(0, 0, kImageBrowserWidth * currentScaleFactor, originSizeF.height() * (kImageBrowserWidth / originSizeF.width()) * currentScaleFactor);
}

QRectF BrowserPage::rect()
{
    switch (currentRotation) {
    case kRotateBy90:
    case kRotateBy270:
        return QRectF(0, 0, static_cast<double>(originSizeF.height() * (kImageBrowserWidth / originSizeF.width()) * currentScaleFactor), static_cast<double>(kImageBrowserWidth * currentScaleFactor));
    default:
        break;
    }

    return QRectF(0, 0, static_cast<double>(kImageBrowserWidth * currentScaleFactor), static_cast<double>(originSizeF.height() * (kImageBrowserWidth / originSizeF.width()) * currentScaleFactor));
}

void BrowserPage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    if (!qFuzzyCompare(renderPixmapScaleFactor, currentScaleFactor)) {
        render(currentScaleFactor, currentRotation);
    }

    if (!viewportRendered && !pixmapHasRendered)
        renderViewPort();

    painter->drawPixmap(0, 0, currentRenderPixmap);   //! m_renderPixmap的大小存在系统缩放，可能不等于option->rect()，需要按坐标绘制

    painter->setPen(Qt::NoPen);

    painter->setBrush(QColor(238, 220, 0, 100));

    painter->setBrush(QColor(59, 148, 1, 100));
}

void BrowserPage::render(const double &scaleFactor, const Rotation &rotation, const bool &renderLater, const bool &force)
{
    if (!force && renderLater && qFuzzyCompare(scaleFactor, scaleFactor) && rotation == currentRotation)
        return;

    currentScaleFactor = scaleFactor;

    if (currentRotation != rotation) {
        currentRotation = rotation;
        if (kRotateBy0 == currentRotation)
            this->setRotation(0);
        else if (kRotateBy90 == currentRotation)
            this->setRotation(90);
        else if (kRotateBy180 == currentRotation)
            this->setRotation(180);
        else if (kRotateBy270 == currentRotation)
            this->setRotation(270);
    }

    if (!renderLater && !qFuzzyCompare(renderPixmapScaleFactor, currentScaleFactor)) {
        renderPixmapScaleFactor = currentScaleFactor;

        if (currentPixmap.isNull()) {
            currentPixmap = QPixmap(static_cast<int>(boundingRect().width() * qApp->devicePixelRatio()),
                                    static_cast<int>(boundingRect().height() * qApp->devicePixelRatio()));
            currentPixmap.fill(Qt::white);
            currentRenderPixmap = currentPixmap;
            currentRenderPixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        } else {
            currentRenderPixmap = currentPixmap.scaled(static_cast<int>(boundingRect().width() * qApp->devicePixelRatio()),
                                                       static_cast<int>(boundingRect().height() * qApp->devicePixelRatio()));
            currentRenderPixmap.setDevicePixelRatio(qApp->devicePixelRatio());
        }

        ++currentPixmapId;

        PageRenderThread::clearImageTasks(docSheet, this, currentPixmapId);
        DocPageNormalImageTask task;

        task.sheet = docSheet;

        task.page = this;

        task.pixmapId = currentPixmapId;

        task.rect = QRect(0, 0,
                          static_cast<int>(boundingRect().width() * qApp->devicePixelRatio()),
                          static_cast<int>(boundingRect().height() * qApp->devicePixelRatio()));

        PageRenderThread::appendTask(task);
    }

    update();
}

void BrowserPage::renderRect(const QRectF &rect)
{
    if (nullptr == sheetBrowserParent)
        return;

    QRect validRect = boundingRect().intersected(rect).toRect();

    DocPageSliceImageTask task;

    task.sheet = docSheet;

    task.page = this;

    task.pixmapId = currentPixmapId;

    task.whole = QRect(0, 0,
                       static_cast<int>(boundingRect().width() * qApp->devicePixelRatio()),
                       static_cast<int>(boundingRect().height() * qApp->devicePixelRatio()));

    task.slice = QRect(static_cast<int>(validRect.x() * qApp->devicePixelRatio()),
                       static_cast<int>(validRect.y() * qApp->devicePixelRatio()),
                       static_cast<int>(validRect.width() * qApp->devicePixelRatio()),
                       static_cast<int>(validRect.height() * qApp->devicePixelRatio()));

    PageRenderThread::appendTask(task);
}

void BrowserPage::renderViewPort()
{
    if (nullptr == sheetBrowserParent)
        return;

    QRect viewPortRect = QRect(0, 0, sheetBrowserParent->size().width(), sheetBrowserParent->size().height());

    QRectF visibleSceneRectF = sheetBrowserParent->mapToScene(viewPortRect).boundingRect();

    QRectF intersectedRectF = this->mapToScene(this->boundingRect()).boundingRect().intersected(visibleSceneRectF);

    //! 如果不在当前可视范围则不加载 强制也没用
    if (intersectedRectF.height() <= 0 && intersectedRectF.width() <= 0)
        return;

    QRectF viewRenderRectF = mapFromScene(intersectedRectF).boundingRect();

    QRect viewRenderRect = QRect(static_cast<int>(viewRenderRectF.x()), static_cast<int>(viewRenderRectF.y()),
                                 static_cast<int>(viewRenderRectF.width()), static_cast<int>(viewRenderRectF.height()));

    //! 扩大加载的视图窗口范围 防止小范围的拖动
    int expand = 200;

    viewRenderRect.setX(viewRenderRect.x() - expand < 0 ? 0 : viewRenderRect.x() - expand);

    viewRenderRect.setY(viewRenderRect.y() - expand < 0 ? 0 : viewRenderRect.y() - expand);

    viewRenderRect.setWidth(viewRenderRect.x() + viewRenderRect.width() + expand * 2 > boundingRect().width() ? viewRenderRect.width() : viewRenderRect.width() + expand * 2);

    viewRenderRect.setHeight(viewRenderRect.y() + viewRenderRect.height() + expand * 2 > boundingRect().height() ? viewRenderRect.height() : viewRenderRect.height() + expand * 2);

    renderRect(viewRenderRect);

    viewportRendered = true;
}

QImage BrowserPage::getCurrentImage(int width, int height)
{
    if (currentPixmap.isNull())
        return QImage();

    //获取图片比原图还大,就不需要原图了
    if (qMin(width, height) > qMax(currentPixmap.width(), currentPixmap.height()))
        return QImage();

    QImage image = currentPixmap.toImage().scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio);

    return image;
}

int BrowserPage::itemIndex()
{
    return currentIndex;
}

void BrowserPage::clearPixmap()
{
    if (renderPixmapScaleFactor < -0.0001)
        return;

    currentPixmap = QPixmap();

    currentRenderPixmap = currentPixmap;

    ++currentPixmapId;

    pixmapHasRendered = false;

    viewportRendered = false;

    renderPixmapScaleFactor = -1;

    PageRenderThread::clearImageTasks(docSheet, this);
}

QPointF BrowserPage::getTopLeftPos()
{
    QPointF p;
    switch (currentRotation) {
    default:
    case kRotateBy0:
        p = pos();
        break;
    case kRotateBy270:
        p.setX(pos().x());
        p.setY(pos().y() - rect().height());
        break;
    case kRotateBy180:
        p.setX(pos().x() - rect().width());
        p.setY(pos().y() - rect().height());
        break;
    case kRotateBy90:
        p.setX(pos().x() - rect().width());
        p.setY(pos().y());
        break;
    }
    return p;
}

void BrowserPage::handleRenderFinished(const int &pixmapId, const QPixmap &pixmap, const QRect &slice)
{
    if (currentPixmapId != pixmapId)
        return;

    if (!slice.isValid()) {   //! 不是切片，整体更新
        pixmapHasRendered = true;
        currentPixmap = pixmap;
    } else {   //! 局部
        QPainter painter(&currentPixmap);
        painter.drawPixmap(slice, pixmap);
    }

    currentRenderPixmap = currentPixmap;

    currentRenderPixmap.setDevicePixelRatio(qApp->devicePixelRatio());

    update();
}
