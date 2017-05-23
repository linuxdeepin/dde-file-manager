#include "wallpaperlist.h"
#include "wallpaperitem.h"
#include "constants.h"
#include "dbus/appearancedaemon_interface.h"

#include <QDebug>
#include <QScrollBar>
#include <QToolButton>
#include <QGSettings>

#include <anchors.h>
#include <dimagebutton.h>

WallpaperList::WallpaperList(QWidget * parent)
    : QListWidget(parent)
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
    setViewMode(QListView::IconMode);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWrapping(false);
    setSelectionRectVisible(false);
    setEditTriggers(QListView::NoEditTriggers);
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);
    horizontalScrollBar()->setEnabled(false);

    connect(&scrollAnimation, &QAbstractAnimation::finished, [this] {
        showDeleteButtonForItem(static_cast<WallpaperItem*>(itemWidget(itemAt(mapFromGlobal(QCursor::pos())))));
        updateBothEndsItem();
    });

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
    QListWidgetItem * item = new QListWidgetItem(this);
    item->setSizeHint(QSize(ItemCellWidth, ItemCellHeight));
    addItem(item);

    WallpaperItem * wallpaper = new WallpaperItem(this, path);
    wallpaper->setFixedSize(item->sizeHint());
    setItemWidget(item, wallpaper);

    connect(wallpaper, &WallpaperItem::pressed, this, &WallpaperList::wallpaperItemPressed);
    connect(wallpaper, &WallpaperItem::hoverIn, this, &WallpaperList::wallpaperItemHoverIn);
    connect(wallpaper, &WallpaperItem::hoverOut, this, &WallpaperList::wallpaperItemHoverOut);
    connect(wallpaper, &WallpaperItem::desktopButtonClicked, this, &WallpaperList::handleSetDesktop);
    connect(wallpaper, &WallpaperItem::lockButtonClicked, this, &WallpaperList::handleSetLock);

    return wallpaper;
}

void WallpaperList::removeWallpaper(const QString &path)
{
    for (int i = 0; i < count(); i++) {
        QListWidgetItem * ii = this->item(i);
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(itemWidget(ii));

        if (wallpaper) {
            if (wallpaper->getPath() == path) {
                removeItemWidget(ii);
                takeItem(i);
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
    scrollList(-gridSize().width() * (width() / gridSize().width() - 2), 500);
}

void WallpaperList::nextPage()
{
    scrollList(gridSize().width() * (width() / gridSize().width() - 2), 500);
}

void WallpaperList::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    int screen_item_count = width() / ItemWidth;

    if (width() % ItemWidth == 0)
        --screen_item_count;

    setGridSize(QSize(width() / screen_item_count, ItemHeight));

    if (gridSize().width() * count() > width())
        QTimer::singleShot(100, this, &WallpaperList::doItemsLayout);

    updateBothEndsItem();
}

void WallpaperList::wheelEvent(QWheelEvent *event)
{
    Q_UNUSED(event);
}

QString WallpaperList::lockWallpaper() const
{
    return m_lockWallpaper;
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
        QListWidgetItem * ii = this->item(i);
        WallpaperItem * wallpaper = qobject_cast<WallpaperItem*>(itemWidget(ii));

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
    WallpaperItem * item = qobject_cast<WallpaperItem*>(sender());

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

    prevItem = static_cast<WallpaperItem*>(itemWidget(itemAt(ItemWidth / 2, ItemHeight / 2)));
    nextItem = static_cast<WallpaperItem*>(itemWidget(itemAt(width() - ItemWidth / 2, ItemHeight / 2)));

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
        emit needCloseButton(item->getPath(), mapToParent(item->geometry().topRight()));
    } else {
        emit needCloseButton("", QPoint(0, 0));
    }
}
