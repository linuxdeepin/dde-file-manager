#include "dbookmarkmountedindicatoritem.h"

#include "app/define.h"

#include "deviceinfo/udisklistener.h"

#include "singleton.h"
#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "windowmanager.h"
#include <QDebug>

DBookmarkMountedIndicatorItem::DBookmarkMountedIndicatorItem(DBookmarkItem *parentItem):
    m_parentItem(parentItem)
{
    init();
}

DBookmarkMountedIndicatorItem::~DBookmarkMountedIndicatorItem()
{

}

void DBookmarkMountedIndicatorItem::init()
{
    setParentItem(m_parentItem);
    setDefaultItem(true);
    setIsMountedIndicator(true);
    boundImageToHover(":/leftsidebar/images/leftsidebar/unmount_hover_16px.png");
    boundImageToPress(":/leftsidebar/images/leftsidebar/unmount_press_16px.png");
    boundImageToRelease(":/leftsidebar/images/leftsidebar/unmount_normal_16px.png");
    boundImageToChecked(":/leftsidebar/images/leftsidebar/unmount_active_16px.png");
    setReleaseBackgroundColor(QColor(Qt::transparent));
    setPressBackgroundColor(QColor(Qt::transparent));
    setHoverBackgroundColor(QColor(Qt::transparent));
    setCheckable(true);
    setHoverBackgroundEnable(true);
    setPressBackgroundEnable(true);
    setDraggable(false);
    setBounds(0, 0, 30, 20);
    setPos(QPoint(160, 5));
    hide();
}

void DBookmarkMountedIndicatorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    DBookmarkItem::paint(painter, option, widget);
}

void DBookmarkMountedIndicatorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setHovered(true);
    update();
}

void DBookmarkMountedIndicatorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return DBookmarkItem::mouseReleaseEvent(event);

    setPress(false);
    setHovered(false);
    deviceListener->unmount(m_parentItem->getDeviceID());
    update();
}

void DBookmarkMountedIndicatorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event)
}

