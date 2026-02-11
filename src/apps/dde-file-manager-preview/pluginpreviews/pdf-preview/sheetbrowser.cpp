// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sheetbrowser.h"
#include "pdfmodel.h"
#include "browserpage.h"
#include "sheetrenderer.h"
#include "docsheet.h"

#include <DGuiApplicationHelper>

#include <QScroller>
#include <QScrollBar>
#include <QTimer>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
SheetBrowser::SheetBrowser(DocSheet *parent)
    : DGraphicsView(parent), docSheet(parent)
{
    setMouseTracking(true);

    setScene(new QGraphicsScene(this));

    setFrameShape(QFrame::NoFrame);

    setAttribute(Qt::WA_TranslucentBackground);

    setBackgroundBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().itemBackground().color()));

    setAttribute(Qt::WA_AcceptTouchEvents);

    //! 捏合缩放
    grabGesture(Qt::PinchGesture);

    scrollerPage = QScroller::scroller(this);

    QScrollBar *vScrollBar = verticalScrollBar();
    QScrollBar *hScrollBar = horizontalScrollBar();

    connect(vScrollBar, &QScrollBar::valueChanged, this, &SheetBrowser::onVerticalScrollBarValueChanged);

    connect(vScrollBar, &QScrollBar::sliderPressed, this, &SheetBrowser::onRemoveDocSlideGesture);

    connect(hScrollBar, &QScrollBar::sliderPressed, this, &SheetBrowser::onRemoveDocSlideGesture);

    vScrollBar->setProperty("_d_slider_spaceUp", 8);
    vScrollBar->setProperty("_d_slider_spaceDown", 8);
    vScrollBar->setAccessibleName("verticalScrollBar");
    hScrollBar->setProperty("_d_slider_spaceLeft", 8);
    hScrollBar->setProperty("_d_slider_spaceRight", 8);
    hScrollBar->setAccessibleName("horizontalScrollBar");
}

SheetBrowser::~SheetBrowser()
{
    qDeleteAll(browserPageList);
}

void SheetBrowser::onVerticalScrollBarValueChanged(int)
{
    beginViewportChange();
}

void SheetBrowser::beginViewportChange()
{
    if (nullptr == viewportChangeTimer) {
        viewportChangeTimer = new QTimer(this);
        connect(viewportChangeTimer, &QTimer::timeout, this, &SheetBrowser::onViewportChanged);
        viewportChangeTimer->setSingleShot(true);
    }

    if (viewportChangeTimer->isActive())
        viewportChangeTimer->stop();

    viewportChangeTimer->start(100);
}

void SheetBrowser::onViewportChanged()
{
    int fromIndex = 0;
    int toIndex = 0;
    currentIndexRange(fromIndex, toIndex);

    foreach (BrowserPage *item, browserPageList) {
        //! 上下多2个浮动
        if (item->itemIndex() < fromIndex - 2 || item->itemIndex() > toIndex + 2) {
            item->clearPixmap();
        }
    }
}

void SheetBrowser::currentIndexRange(int &fromIndex, int &toIndex)
{
    fromIndex = -1;
    toIndex = -1;

    int value = verticalScrollBar()->value();

    for (int i = 0; i < browserPageList.count(); ++i) {
        int y = 0;

        switch (docSheet->operation().rotation) {
        case kRotateBy0:
        case kRotateBy90:
            y = static_cast<int>(browserPageList[i]->y());
            break;
        case kRotateBy180:
            y = static_cast<int>(browserPageList[i]->y() - browserPageList[i]->boundingRect().height());
            break;
        case kRotateBy270:
            y = static_cast<int>(browserPageList[i]->y() - browserPageList[i]->boundingRect().width());
            break;
        default:
            break;
        }

        if (-1 == fromIndex && y + browserPageList[i]->rect().height() >= value) {
            fromIndex = i;
        }

        if (-1 == toIndex && y >= (value + this->height())) {
            toIndex = i - 1;
            break;
        }

        if (i == (browserPageList.count() - 1)) {
            toIndex = i;
        }
    }
}

void SheetBrowser::onRemoveDocSlideGesture()
{
    scrollerPage->stop();
}

void SheetBrowser::onInit()
{
    if (1 != jumpPageNumber) {
        setCurrentPage(jumpPageNumber);
        jumpPageNumber = 1;
    }

    onViewportChanged();
}

void SheetBrowser::deform(SheetOperation &operation)
{
    lastScaleFactor = operation.scaleFactor;

    int page = operation.currentPage;
    //! 进行render 并算出最宽的一行
    double maxWidth = 0;   //! 最宽的一行
    double maxHeight = 0;   //! 总高度
    int space = 5;   //! 页之间间隙

    for (int i = 0; i < browserPageList.count(); ++i) {
        if (i % 2 == 1)
            continue;

        int j = i + 1;

        browserPageList.at(i)->render(operation.scaleFactor, operation.rotation, true);

        if (j < browserPageList.count())
            browserPageList.at(j)->render(operation.scaleFactor, operation.rotation, true);

        if (SinglePageMode == operation.layoutMode) {
            if (browserPageList.at(i)->rect().width() > maxWidth)
                maxWidth = static_cast<int>(browserPageList.at(i)->rect().width());

            if (j < browserPageList.count()) {
                if (browserPageList.at(j)->rect().width() > maxWidth)
                    maxWidth = static_cast<int>(browserPageList.at(j)->rect().width());
            }

        } else if (TwoPagesMode == operation.layoutMode) {
            if (j < browserPageList.count()) {
                if (static_cast<int>(browserPageList.at(i)->rect().width() + browserPageList.at(i + 1)->rect().width()) > maxWidth)
                    maxWidth = static_cast<int>(browserPageList.at(i)->rect().width() + browserPageList.at(i + 1)->rect().width());
            } else {
                if (static_cast<int>(browserPageList.at(i)->rect().width()) * 2 > maxWidth) {
                    maxWidth = static_cast<int>(browserPageList.at(i)->rect().width()) * 2;
                }
            }
        }
    }

    if (SinglePageMode == operation.layoutMode) {
        for (int i = 0; i < browserPageList.count(); ++i) {
            int x = static_cast<int>(maxWidth - browserPageList.at(i)->rect().width()) / 2;
            if (x < 0)
                x = 0;

            if (kRotateBy0 == operation.rotation)
                browserPageList.at(i)->setPos(x, maxHeight);
            else if (kRotateBy90 == operation.rotation)
                browserPageList.at(i)->setPos(x + browserPageList.at(i)->boundingRect().height(), maxHeight);
            else if (kRotateBy180 == operation.rotation)
                browserPageList.at(i)->setPos(x + browserPageList.at(i)->boundingRect().width(), maxHeight + browserPageList.at(i)->boundingRect().height());
            else if (kRotateBy270 == operation.rotation)
                browserPageList.at(i)->setPos(x, maxHeight + browserPageList.at(i)->boundingRect().width());

            maxHeight += browserPageList.at(i)->rect().height() + space;
        }
    } else if (TwoPagesMode == operation.layoutMode) {
        for (int i = 0; i < browserPageList.count(); ++i) {
            if (i % 2 == 1)
                continue;

            int x = static_cast<int>(maxWidth / 2 - browserPageList.at(i)->rect().width());

            if (kRotateBy0 == operation.rotation) {
                browserPageList.at(i)->setPos(x, maxHeight);
                if (browserPageList.count() > i + 1) {
                    browserPageList.at(i + 1)->setPos(x + space + browserPageList.at(i)->rect().width(), maxHeight);
                }
            } else if (kRotateBy90 == operation.rotation) {
                browserPageList.at(i)->setPos(x + browserPageList.at(i)->boundingRect().height(), maxHeight);
                if (browserPageList.count() > i + 1) {
                    browserPageList.at(i + 1)->setPos(x + space + browserPageList.at(i)->rect().width() + browserPageList.at(i + 1)->boundingRect().height(), maxHeight);
                }
            } else if (kRotateBy180 == operation.rotation) {
                browserPageList.at(i)->setPos(x + browserPageList.at(i)->boundingRect().width(), maxHeight + browserPageList.at(i)->boundingRect().height());
                if (browserPageList.count() > i + 1) {
                    browserPageList.at(i + 1)->setPos(x + space + browserPageList.at(i)->rect().width() + browserPageList.at(i + 1)->boundingRect().width(), maxHeight + browserPageList.at(i + 1)->boundingRect().height());
                }
            } else if (kRotateBy270 == operation.rotation) {
                browserPageList.at(i)->setPos(x, maxHeight + browserPageList.at(i)->boundingRect().width());
                if (browserPageList.count() > i + 1) {
                    browserPageList.at(i + 1)->setPos(x + space + browserPageList.at(i)->rect().width(), maxHeight + browserPageList.at(i + 1)->boundingRect().width());
                }
            }

            if (browserPageList.count() > i + 1)
                maxHeight += qMax(browserPageList.at(i)->rect().height(), browserPageList.at(i + 1)->rect().height()) + space;
            else
                maxHeight += browserPageList.at(i)->rect().height() + space;
        }
        maxWidth += space;
    }

    setSceneRect(0, 0, maxWidth, maxHeight);

    if (page > 0 && page <= browserPageList.count()) {
        verticalScrollBar()->setValue(static_cast<int>(browserPageList[page - 1]->getTopLeftPos().y()));
        horizontalScrollBar()->setValue(static_cast<int>(browserPageList[page - 1]->getTopLeftPos().x()));
    }

    lastrotation = operation.rotation;

    beginViewportChange();
}

bool SheetBrowser::hasLoaded()
{
    return hasLoadedPage;
}

void SheetBrowser::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void SheetBrowser::focusOutEvent(QFocusEvent *event)
{
    DGraphicsView::focusOutEvent(event);
}

void SheetBrowser::timerEvent(QTimerEvent *event)
{
    QGraphicsView::timerEvent(event);
    if (event->timerId() == repeatTimer.timerId()) {
        repeatTimer.stop();
        canTouchScreen = false;
    }
}

void SheetBrowser::wheelEvent(QWheelEvent *e)
{
    QPointF centerPoint = rect().center();
    BrowserPage *page = getBrowserPageForPoint(centerPoint);
    if (page)
        emit sigPageChanged(page->itemIndex() + 1);

    QGraphicsView::wheelEvent(e);
}

int SheetBrowser::allPages()
{
    return browserPageList.count();
}

int SheetBrowser::currentPage()
{
    if (currentPageIndex >= 1)
        return currentPageIndex;

    return currentScrollValueForPage();
}

int SheetBrowser::currentScrollValueForPage()
{
    int value = verticalScrollBar()->value();

    int index = 0;

    for (int i = 0; i < browserPageList.count(); ++i) {
        int y = 0;

        if (kRotateBy0 == docSheet->operation().rotation) {
            y = static_cast<int>(browserPageList[i]->y());
        } else if (kRotateBy90 == docSheet->operation().rotation) {
            y = static_cast<int>(browserPageList[i]->y());
        } else if (kRotateBy180 == docSheet->operation().rotation) {
            y = static_cast<int>(browserPageList[i]->y() - browserPageList[i]->boundingRect().height());
        } else if (kRotateBy270 == docSheet->operation().rotation) {
            y = static_cast<int>(browserPageList[i]->y() - browserPageList[i]->boundingRect().width());
        }

        if (y + browserPageList[i]->rect().height() >= value) {
            index = i;
            break;
        }
    }

    return index + 1;
}

void SheetBrowser::setCurrentPage(int page)
{
    if (page < 1 || page > allPages())
        return;

    needNotifyCurPageChanged = false;

    if (kRotateBy0 == docSheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().x()));
        verticalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().y()));
    } else if (kRotateBy90 == docSheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().x() - browserPageList.at(page - 1)->boundingRect().height()));
        verticalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().y()));
    } else if (kRotateBy180 == docSheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().x() - browserPageList.at(page - 1)->boundingRect().width()));
        verticalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().y() - browserPageList.at(page - 1)->boundingRect().height()));
    } else if (kRotateBy270 == docSheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().x()));
        verticalScrollBar()->setValue(static_cast<int>(browserPageList.at(page - 1)->pos().y() - browserPageList.at(page - 1)->boundingRect().width()));
    }

    needNotifyCurPageChanged = true;
}

bool SheetBrowser::getExistImage(int index, QImage &image, int width, int height)
{
    if (browserPageList.count() <= index)
        return false;

    image = browserPageList.at(index)->getCurrentImage(width, height);

    return !image.isNull();
}

BrowserPage *SheetBrowser::getBrowserPageForPoint(QPointF &viewPoint)
{
    BrowserPage *item = nullptr;

    QPoint ponit = viewPoint.toPoint();

    const QList<QGraphicsItem *> &itemlst = this->items(ponit);

    for (QGraphicsItem *itemIter : itemlst) {
        item = dynamic_cast<BrowserPage *>(itemIter);

        if (item != nullptr) {
            const QPointF &itemPoint = item->mapFromScene(mapToScene(ponit));

            if (item->contains(itemPoint)) {
                viewPoint = itemPoint;
                return item;
            }
        }
    }

    return nullptr;
}

qreal SheetBrowser::maxWidth()
{
    return maxPageWidth;
}

qreal SheetBrowser::maxHeight()
{
    return maxPageHeight;
}

void SheetBrowser::showEvent(QShowEvent *event)
{
    QTimer::singleShot(100, this, SLOT(onInit()));

    QGraphicsView::showEvent(event);
}

QList<BrowserPage *> SheetBrowser::pages()
{
    return browserPageList;
}

void SheetBrowser::init(SheetOperation &operation)
{
    int pageCount = docSheet->pageCount();

    for (int i = 0; i < pageCount; ++i) {
        BrowserPage *item = new BrowserPage(this, i, docSheet);

        browserPageList.append(item);

        if (docSheet->renderer()->getPageSize(i).width() > maxPageWidth)
            maxPageWidth = docSheet->renderer()->getPageSize(i).width();

        if (docSheet->renderer()->getPageSize(i).height() > maxPageHeight)
            maxPageHeight = docSheet->renderer()->getPageSize(i).height();

        scene()->addItem(item);
    }

    deform(operation);

    jumpPageNumber = operation.currentPage;

    hasLoadedPage = true;
}

void SheetBrowser::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        isPressed = true;
        mouseStartPos = e->globalPos();
        QWidget *pWidget = getMainDialog();
        if (pWidget)
            windowStartPos = pWidget->frameGeometry().topLeft();
    }
}

void SheetBrowser::mouseMoveEvent(QMouseEvent *e)
{
    if (isPressed) {
        QPoint mouseMoveDistance = e->globalPos() - mouseStartPos;
        QWidget *pWidget = getMainDialog();
        if (pWidget)
            pWidget->move(windowStartPos + mouseMoveDistance);
    }
}

void SheetBrowser::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        isPressed = false;
}
