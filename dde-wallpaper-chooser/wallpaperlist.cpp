#include "wallpaperlist.h"
#include "wallpaperitem.h"
#include "constants.h"
#include "dbus/appearancedaemon_interface.h"

#include <QDebug>
#include <QScrollBar>
#include <QToolButton>
#include <QGSettings>
#include <QHBoxLayout>

#include <anchors.h>
#include <dimagebutton.h>

WallpaperList::WallpaperList(QWidget * parent)
    : QScrollArea(parent)
    , m_dbusAppearance(new AppearanceDaemonInterface(AppearanceServ,
                                                   AppearancePath,
                                                   QDBusConnection::sessionBus(),
                                                   this))
    , m_wmInter(new com::deepin::wm("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this))
    , prevButton(new DImageButton(":/images/previous_normal.png",
                                  ":/images/previous_hover.png",
                                  ":/images/previous_press.png", this))
    , nextButton(new DImageButton(":/images/next_normal.png",
                                  ":/images/next_hover.png",
                                  ":/images/next_press.png", this))
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("WallpaperList{background: transparent;}");
    setFrameShape(QFrame::NoFrame);
    horizontalScrollBar()->setEnabled(false);

    connect(&scrollAnimation, &QAbstractAnimation::finished, [this] {
        showDeleteButtonForItem(static_cast<WallpaperItem*>(itemAt(mapFromGlobal(QCursor::pos()))));
        updateBothEndsItem();
    });

    m_contentWidget = new QWidget(this);

    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_contentLayout = new QHBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    setWidget(m_contentWidget);
    m_contentWidget->setAutoFillBackground(false);

    prevButton->hide();
    prevButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    prevButton.setAnchor(Qt::AnchorLeft, this, Qt::AnchorLeft);
    nextButton->hide();
    nextButton.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
    nextButton.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);

    connect(prevButton.widget(), &DImageButton::clicked, this, &WallpaperList::prevPage);
    connect(nextButton.widget(), &DImageButton::clicked, this, &WallpaperList::nextPage);

    scrollAnimation.setTargetObject(horizontalScrollBar());
    scrollAnimation.setPropertyName("value");
    scrollAnimation.setDuration(QEasingCurve::OutExpo);
}

WallpaperList::~WallpaperList()
{

}

WallpaperItem * WallpaperList::addWallpaper(const QString &path)
{
    WallpaperItem * wallpaper = new WallpaperItem(this, path);
    wallpaper->setFixedSize(QSize(ItemWidth, ItemHeight));
    addItem(wallpaper);

    connect(wallpaper, &WallpaperItem::pressed, this, &WallpaperList::wallpaperItemPressed);
    connect(wallpaper, &WallpaperItem::hoverIn, this, &WallpaperList::wallpaperItemHoverIn);
    connect(wallpaper, &WallpaperItem::hoverOut, this, &WallpaperList::wallpaperItemHoverOut);
    connect(wallpaper, &WallpaperItem::desktopButtonClicked, this, &WallpaperList::handleSetDesktop);
    connect(wallpaper, &WallpaperItem::lockButtonClicked, this, &WallpaperList::handleSetLock);

    return wallpaper;
}

WallpaperItem *WallpaperList::getWallpaperByPath(const QString &path) const
{
    for (int i = 0; i < count(); i++) {
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(this->item(i));

        if (wallpaper) {
            if (wallpaper->getPath() == path) {
                return wallpaper;
            }
        }
    }

    return 0;
}

void WallpaperList::removeWallpaper(const QString &path)
{
    for (int i = 0; i < count(); i++) {
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(this->item(i));

        if (wallpaper) {
            if (wallpaper->getPath() == path) {
                removeItem(wallpaper);
                break;
            }
        }
    }

    updateBothEndsItem();
}

void WallpaperList::scrollList(int step, int duration)
{
    if (scrollAnimation.state() == QAbstractAnimation::Running)
        return;

    int start_value = horizontalScrollBar()->value();
    int end_value = start_value + step;

    if ((end_value < horizontalScrollBar()->minimum() && start_value == horizontalScrollBar()->minimum())
            || (end_value > horizontalScrollBar()->maximum() && start_value == horizontalScrollBar()->maximum())) {
        return;
    }

    scrollAnimation.setDuration(duration);
    scrollAnimation.setStartValue(start_value);
    scrollAnimation.setEndValue(end_value);
    scrollAnimation.start();

    prevButton->hide();
    nextButton->hide();

    if (prevItem)
        prevItem->setOpacity(1);

    if (nextItem)
        nextItem->setOpacity(1);

    // hide the delete button.
    emit needCloseButton("", QPoint(0, 0));
}

void WallpaperList::prevPage()
{
    int c = width() / gridSize().width();

    scrollList((2 - c) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::nextPage()
{
    int c = width() / gridSize().width();

    scrollList((c - 2) * (m_contentLayout->spacing() + ItemWidth), 500);
}

void WallpaperList::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    int screen_item_count = width() / ItemWidth;

    if (width() % ItemWidth == 0)
        --screen_item_count;

    setGridSize(QSize(width() / screen_item_count, ItemHeight));
}

void WallpaperList::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
}

void WallpaperList::showEvent(QShowEvent *event)
{
    updateBothEndsItem();

    return QScrollArea::showEvent(event);
}

QString WallpaperList::lockWallpaper() const
{
    return m_lockWallpaper;
}

QSize WallpaperList::gridSize() const
{
    return m_gridSize;
}

void WallpaperList::setGridSize(const QSize &size)
{
    if (m_gridSize == size)
        return;

    int c = width() / size.width();

    m_gridSize = size;
    m_contentLayout->setSpacing((width() - c * ItemWidth) / (c + 1));
    m_contentLayout->setContentsMargins(m_contentLayout->spacing(), 0,
                                        m_contentLayout->spacing(), 0);
    m_contentWidget->adjustSize();
}

void WallpaperList::addItem(QWidget *item)
{
    m_contentLayout->addWidget(item);
    m_contentWidget->adjustSize();
}

QWidget *WallpaperList::item(int index) const
{
    if (index >= m_contentLayout->count())
        return 0;

    return m_contentLayout->itemAt(index)->widget();
}

QWidget *WallpaperList::itemAt(const QPoint &pos) const
{
    return itemAt(pos.x(), pos.y());
}

QWidget *WallpaperList::itemAt(int x, int y) const
{
    Q_UNUSED(y)

    return item((horizontalScrollBar()->value() + x) / gridSize().width());
}

void WallpaperList::removeItem(QWidget *item)
{
    for (int i = 0; i < m_contentLayout->count(); ++i) {
        if (m_contentLayout->itemAt(i)->widget() == item) {
            removeItem(i);
            break;
        }
    }
}

void WallpaperList::removeItem(int index)
{
    QLayoutItem *item = m_contentLayout->takeAt(index);
    QWidget *w = item->widget();

    if (w) {
        w->hide();
        w->deleteLater();
    }

    m_contentWidget->adjustSize();
}

int WallpaperList::count() const
{
    return m_contentLayout->count();
}

void WallpaperList::clear()
{
    while (m_contentLayout->count() > 0) {
        QLayoutItem *item = m_contentLayout->takeAt(0);
        QWidget *w = item->widget();

        if (w)
            w->deleteLater();
    }
}

QString WallpaperList::desktopWallpaper() const
{
    return m_desktopWallpaper;
}

void WallpaperList::wallpaperItemPressed()
{
    WallpaperItem * item = qobject_cast<WallpaperItem*>(sender());

    if (item == prevItem || item == nextItem)
        return;

    for (int i = 0; i < count(); i++) {
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(this->item(i));

        if (wallpaper) {
            if (wallpaper == item) {
                wallpaper->slideUp();

                const QString &path = wallpaper->getPath();
                emit needPreviewWallpaper(path);

                m_desktopWallpaper = path;
                m_lockWallpaper = path;
            } else {
                wallpaper->slideDown();
            }
        }
    }
}

void WallpaperList::wallpaperItemHoverIn()
{
    WallpaperItem *item = qobject_cast<WallpaperItem*>(sender());

    if (item->isVisible())
        showDeleteButtonForItem(item);
}

void WallpaperList::wallpaperItemHoverOut()
{
//    emit needCloseButton("", QPoint(0, 0));
}

void WallpaperList::handleSetDesktop()
{
    m_lockWallpaper = "";

    qDebug() << "desktop item set, quit";
    if (parentWidget())
        parentWidget()->hide();
}

void WallpaperList::handleSetLock()
{
    m_desktopWallpaper = "";

    qDebug() << "lock item set, quit";
    if (parentWidget())
        parentWidget()->hide();
}

void WallpaperList::updateBothEndsItem()
{
    int current_value = horizontalScrollBar()->value();

    if (prevItem)
        prevItem->setOpacity(1);

    if (nextItem)
        nextItem->setOpacity(1);

    prevItem = qobject_cast<WallpaperItem*>(itemAt(ItemWidth / 2, ItemHeight / 2));
    nextItem = qobject_cast<WallpaperItem*>(itemAt(width() - ItemWidth / 2, ItemHeight / 2));

    if (current_value == horizontalScrollBar()->minimum()) {
        prevItem = Q_NULLPTR;
    }

    if (current_value == horizontalScrollBar()->maximum()) {
        nextItem = Q_NULLPTR;
    }

    if (prevItem) {
        prevButton.setLeftMargin(gridSize().width() / 2 - prevButton->sizeHint().width() / 2);
        prevItem->setOpacity(0.4);
    }

    prevButton->setVisible(prevItem);

    if (nextItem) {
        nextButton.setRightMargin(gridSize().width() / 2 - nextButton->sizeHint().width() / 2);
        nextItem->setOpacity(0.4);
    }

    nextButton->setVisible(nextItem);
}

void WallpaperList::showDeleteButtonForItem(const WallpaperItem *item) const
{
    if (item && item->getDeletable()) {
        emit needCloseButton(item->getPath(), item->mapTo(parentWidget(), item->conentImageGeometry().topRight()));
    } else {
        emit needCloseButton("", QPoint(0, 0));
    }
}
