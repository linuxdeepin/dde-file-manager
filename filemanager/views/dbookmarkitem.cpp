#include "dbookmarkitem.h"
#include "dbookmarkitemgroup.h"
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "dfilemenu.h"
#include "filemenumanager.h"

#include "../../deviceinfo/deviceinfo.h"
#include "../models/bookmark.h"
#include "../app/global.h"
#include "windowmanager.h"
#include "../app/fmevent.h"

#include "windowmanager.h"
#include "ddragwidget.h"


DBookmarkItem::DBookmarkItem()
{
    init();
    m_isDefault = true;
}

DBookmarkItem::DBookmarkItem(DeviceInfo *deviceInfo)
{
    init();
    m_isDisk = true;
    m_checkable = true;
    m_url = DUrl::fromLocalFile(deviceInfo->getMountPath());
    m_isDefault = true;
    m_sysPath = deviceInfo->getSysPath();
    m_textContent = deviceInfo->displayName();
    boundImageToHover(":/icons/images/icons/disk_hover_16px.svg");
    boundImageToPress(":/icons/images/icons/disk_hover_16px.svg");
    boundImageToRelease(":/icons/images/icons/disk_normal_16px.svg");
    boundBigImageToHover(":/icons/images/icons/disk_hover_22px.svg");
    boundBigImageToPress(":/icons/images/icons/disk_hover_22px.svg");
    boundBigImageToRelease(":/icons/images/icons/disk_normal_22px.svg");
    if(!m_url.isEmpty())
        m_isMounted = true;
}

DBookmarkItem::DBookmarkItem(BookMark *bookmark)
{
    init();
    m_url = bookmark->getUrl();
    m_textContent = bookmark->getName();
    boundImageToHover(":/icons/images/icons/bookmarks_hover_16px.svg");
    boundImageToPress(":/icons/images/icons/bookmarks_checked_16px.svg");
    boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
    boundBigImageToHover(":/icons/images/icons/favourite_hover.svg");
    boundBigImageToPress(":/icons/images/icons/favourite_checked.svg");
    boundBigImageToRelease(":/icons/images/icons/favourite_normal.svg");
}

void DBookmarkItem::init()
{
    //setFlag(QGraphicsItem::ItemIsFocusable);
    //setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setAcceptHoverEvents(true);
    setReleaseBackgroundColor(QColor(238,232,205,0));
    setPressBackgroundColor(QColor(44,167,248,255));
    setHoverBackgroundColor(Qt::lightGray);
    setPressBackgroundEnable(true);
    setReleaseBackgroundEnable(true);
    setHoverBackgroundEnable(true);
    setTextColor(Qt::black);
    setAcceptDrops(true);
    m_checkable = true;
}

QRectF DBookmarkItem::boundingRect() const
{
    return QRectF(m_x_axis - m_adjust,
                  m_y_axis - m_adjust,
                  m_width + m_adjust,
                  m_height + m_adjust);
}

void DBookmarkItem::setTightMode(bool v)
{
    m_isTightMode = v;
}

bool DBookmarkItem::isTightModel()
{
    return m_isTightMode;
}

void DBookmarkItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    double w = m_width;
    QColor textColor;
    double leftPadding = 18;
    double dy;
    QPixmap press;
    QPixmap release;
    QPixmap hover;
    if(m_isTightMode)
    {
        press = m_pressImageBig;
        release = m_releaseImageBig;
        hover = m_hoverImageBig;
        dy = m_height/2 - m_releaseImageBig.height()/2;
    }
    else
    {
        press = m_pressImage;
        release = m_releaseImage;
        hover = m_hoverImage;
        dy = m_height/2 - m_releaseImage.height()/2;
    }
    if(m_pressed || (m_checked && m_checkable))
    {
        if(m_pressBackgroundEnabled)
        {
            painter->setPen(QColor(0,0,0,0));
            painter->setBrush(m_pressBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
            textColor = Qt::white;
        }
        painter->drawPixmap(leftPadding, dy, press.width(), press.height(), press);
    }
    else if(m_hovered)
    {
        if(m_hoverBackgroundEnabled)
        {
            painter->setBrush(m_hoverBackgroundColor);
            painter->setPen(QColor(0,0,0,0));
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
        }
        painter->drawPixmap(leftPadding, dy,  hover.width(), hover.height(), hover);
        textColor = Qt::white;
    }
    else
    {
        if(m_releaseBackgroundEnabled)
        {
            painter->setPen(QColor(0,0,0,0));
            painter->setBrush(m_releaseBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
        }
        painter->drawPixmap(leftPadding, dy, release.width(), release.height(), release);
        textColor = Qt::black;
    }

    double textTopPadding = 4;
    if(!m_isTightMode)
    {
        painter->setPen(textColor);
        painter->setFont(m_font);
        QRect rect(leftPadding + 24, m_y_axis + m_height/4, m_width - 25, m_height);
        painter->drawText(rect,Qt::TextWordWrap|Qt::AlignLeft,m_textContent);
    }
    else
    {
        if(!m_isDefault && !m_textContent.isEmpty())
        {
            QString text = m_textContent.at(0);
            painter->setPen(QColor(0x2CA7F8));
            QFont font;
            font.setPointSize(8);
            painter->setFont(font);
            painter->drawText(leftPadding + 6, m_y_axis + m_height*3/4 - textTopPadding, text);
        }
    }

    if(m_isMounted)
    {
        QPixmap pressPic(":/icons/images/icons/unmount_press.svg");
        QPixmap normalPic(":/icons/images/icons/unmount_normal.svg");
        QPixmap hoverPic(":/icons/images/icons/unmount_press.svg");
        if(m_pressed)
            painter->drawPixmap(w/2 - leftPadding,
                                -pressPic.height()/2,
                                pressPic.width(),
                                pressPic.height(),
                                pressPic);
        else if(m_hovered)
            painter->drawPixmap(w/2 - leftPadding,
                                -hoverPic.height()/2,
                                hoverPic.width(),
                                hoverPic.height(),
                                hoverPic);
        else
            painter->drawPixmap(w/2 - leftPadding,
                                -normalPic.height()/2,
                                normalPic.width(),
                                normalPic.height(),
                                normalPic);

    }
}

void DBookmarkItem::boundImageToPress(QString imagePath)
{
    m_pressImage.load(imagePath);
}

void DBookmarkItem::boundImageToRelease(QString imagePath)
{
    m_releaseImage.load(imagePath);
}

void DBookmarkItem::boundImageToHover(QString imagePath)
{
    m_hoverImage.load(imagePath);
}

void DBookmarkItem::boundBigImageToPress(QString imagePath)
{
    m_pressImageBig.load(imagePath);
}

void DBookmarkItem::boundBigImageToRelease(QString imagePath)
{
    m_releaseImageBig.load(imagePath);
}

void DBookmarkItem::boundBigImageToHover(QString imagePath)
{
    m_hoverImageBig.load(imagePath);
}

void DBookmarkItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_isDraggable)
        {
            m_xPress = event->pos().x();
            m_yPress = event->pos().y();
        }
        m_pressed = true;
        emit clicked();
        if(m_group)
        {
            m_group->deselectAll();
            setChecked(true);
        }
    }
    update();
}

void DBookmarkItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_pressed&&m_isDraggable)
    {
        m_xOffset = event->pos().x() - m_xPress;
        m_xPos += m_xOffset;
        m_xPress = event->pos().x();

        m_yOffset = event->pos().y() - m_yPress;
        m_yPos += m_yOffset;
        m_yPress = event->pos().y();
        moveBy(m_xPos,m_yPos);
        update();
    }

    if(m_pressed && !m_isDefault)
    {
        if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
                .length() < QApplication::startDragDistance()) {
                return;
            }

        drag = new DDragWidget((QObject*)event->widget());
        QMimeData *mimeData = new QMimeData;
        drag->setPixmap(toPixmap());
        drag->setMimeData(mimeData);
        drag->setHotSpot(QPoint(m_width/4, 4));
        drag->startDrag();
        m_pressed = false;
        drag = NULL;
        QPoint point = scene()->views().at(0)->mapFromGlobal(QCursor::pos());
        QPointF point1 = scene()->views().at(0)->mapToScene(point);
        emit dragFinished(QCursor::pos(), point1, this);
    }
}

QPixmap DBookmarkItem::toPixmap()
{
    if (!scene()) {
        return QPixmap();
    }

//    QRectF r = boundingRect();
//    QPixmap pixmap(r.width(), r.height());
//    pixmap.fill(QColor(0, 0, 0, 0));
//    QPainter painter(&pixmap);
//    painter.drawRect(r);
//    scene()->render(&painter, QRectF(), sceneBoundingRect());
//    painter.end();
    QPixmap pixmap(":/icons/images/icons/reordering_line.svg");
    if(m_isTightMode)
        return pixmap.scaled(60, pixmap.height(), Qt::IgnoreAspectRatio);
    else
        return pixmap.scaledToWidth(200);
}

bool DBookmarkItem::isMounted()
{
    return m_isMounted;
}

void DBookmarkItem::setMounted(bool v)
{
    m_isMounted = v;
}

void DBookmarkItem::setDeviceLabel(const QString &label)
{
    m_deviceLabel = label;
}

QString DBookmarkItem::getDeviceLabel()
{
    return m_deviceLabel;
}

void DBookmarkItem::setSysPath(const QString &path)
{
    m_sysPath = path;
}

QString DBookmarkItem::getSysPath()
{
    return m_sysPath;
}

int DBookmarkItem::windowId()
{
    return WindowManager::getWindowId(scene()->views().at(0)->window());
}

DBookmarkItem *DBookmarkItem::makeBookmark(const QString &name, const DUrl &url)
{
    DBookmarkItem * item = new DBookmarkItem;
    item->setDefaultItem(false);
    item->setText(name);
    item->setUrl(url);
    item->boundImageToHover(":/icons/images/icons/bookmarks_hover_16px.svg");
    item->boundImageToPress(":/icons/images/icons/bookmarks_checked_16px.svg");
    item->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
    item->boundBigImageToHover(":/icons/images/icons/favourite_hover.svg");
    item->boundBigImageToPress(":/icons/images/icons/favourite_checked.svg");
    item->boundBigImageToRelease(":/icons/images/icons/favourite_normal.svg");
    return item;
}

QSizeF DBookmarkItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return QSizeF(m_width, m_height);
}

void DBookmarkItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_group && m_pressed)
    {
        FMEvent e;
        e = windowId();
        if(m_url.isBookMarkFile())
            e = DUrl::fromLocalFile(m_url.path());
        else
            e = m_url;
        e = FMEvent::LeftSideBar;
        emit m_group->url(e);
    }
    m_pressed = false;
    update();
}

void DBookmarkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}

void DBookmarkItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    m_hovered = true;
    event->accept();
    emit dragEntered();
    //QGraphicsItem::dragEnterEvent(event);
    update();
}

void DBookmarkItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    emit dragLeft();
    QGraphicsItem::dragLeaveEvent(event);
    update();
}

void DBookmarkItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_hovered = true;
    event->accept();
}

void DBookmarkItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();
    emit dropped();
    if(!event->mimeData()->hasUrls())
        return;
    if(!m_isDefault)
        return;
    FMEvent e;
    e = FMEvent::LeftSideBar;
    e = m_url;
    e = windowId();
    fileService->pasteFile(AbstractFileController::CopyType, DUrl::fromQUrlList(event->mimeData()->urls()), e);
    QGraphicsItem::dropEvent(event);
}

bool DBookmarkItem::eventFilter(QObject *obj, QEvent *e)
{
    qDebug() << obj << e;
    return false;
}

void DBookmarkItem::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void DBookmarkItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    event->accept();
    m_hovered = true;
    update();
}

void DBookmarkItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    event->accept();
    if(!m_isMenuOpened)
    {
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    m_isMenuOpened = true;
    DFileMenu *menu;

    if (m_url.isRecentFile()){
        menu = FileMenuManager::createRecentLeftBarMenu();
    }
    else if (m_url.isTrashFile()){
    menu = FileMenuManager::createTrashLeftBarMenu();
    }
    else if(m_isDefault)
        menu = FileMenuManager::createDefaultBookMarkMenu();
    else if(m_isDisk)
        menu = FileMenuManager::createDiskLeftBarMenu();
    else
        menu = FileMenuManager::createCustomBookMarkMenu();
    QWidget* window = scene()->views().at(0)->window();

    menu->setWindowId(WindowManager::getWindowId(window));
    DUrlList urls;
    urls.append(m_url);
    menu->setUrls(urls);

    QPointer<DBookmarkItem> me = this;

    menu->exec();
    menu->deleteLater();

    if(me) {
        m_isMenuOpened = false;
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::setDraggable(bool b)
{
    m_isDraggable = b;
}

void DBookmarkItem::setBounds(int x, int y, int w, int h)
{
    m_x_axis = x;
    m_y_axis = y;
    m_width = w;
    m_height = h;
}

void DBookmarkItem::setPressBackgroundColor(const QColor &color)
{
    m_pressBackgroundColor = color;
}

void DBookmarkItem::setReleaseBackgroundColor(const QColor &color)
{
   m_releaseBackgroundColor = color;
}

void DBookmarkItem::setHoverBackgroundColor(const QColor &color)
{
    m_hoverBackgroundColor = color;
}

void DBookmarkItem::setHoverEnableFlag(bool flag)
{
     setAcceptHoverEvents(flag);
}

void DBookmarkItem::setAdjust(qreal value)
{
    m_adjust = value;
}

bool DBookmarkItem::isPressed()
{
    return m_pressed;
}

bool DBookmarkItem::isHovered()
{
    return m_hovered;
}

void DBookmarkItem::setBackgroundEnable(bool flag)
{
    m_backgroundEnabled = flag;
}

int DBookmarkItem::boundX()
{
    return m_x_axis;
}

int DBookmarkItem::boundY()
{
    return m_y_axis;
}

int DBookmarkItem::boundWidth()
{
    return m_width;
}

int DBookmarkItem::boundHeight()
{
    return m_height;
}

void DBookmarkItem::setText(const QString & text)
{
    m_textContent = text;
}

QString DBookmarkItem::text()
{
    return m_textContent;
}

void DBookmarkItem::setTextColor(const QColor &color)
{
    m_textColor = color;
}

QColor DBookmarkItem::getTextColor()
{
    return m_textColor;
}

void DBookmarkItem::setPress(bool b)
{
    m_pressed = b;
}

void DBookmarkItem::setPressBackgroundEnable(bool b)
{
    m_pressBackgroundEnabled = b;
}

void DBookmarkItem::setReleaseBackgroundEnable(bool b)
{
    m_releaseBackgroundEnabled = b;
}

void DBookmarkItem::setHoverBackgroundEnable(bool b)
{
    m_hoverBackgroundEnabled = b;
}

void DBookmarkItem::setUrl(const DUrl &url)
{
    m_url = url;
}

DUrl DBookmarkItem::getUrl()
{
    return m_url;
}

void DBookmarkItem::setCheckable(bool b)
{
    m_checkable = b;
}

void DBookmarkItem::setChecked(bool b)
{
    m_checked = b;
    update();
}

bool DBookmarkItem::isChecked()
{
    return m_checked;
}

void DBookmarkItem::setItemGroup(DBookmarkItemGroup *group)
{
    m_group = group;
}

void DBookmarkItem::setWidth(double w)
{
    m_width = w;
}

void DBookmarkItem::setHeight(double h)
{
    m_height = h;
}

void DBookmarkItem::setDefaultItem(bool v)
{
    m_isDefault = v;
}

bool DBookmarkItem::isDefaultItem()
{
    return m_isDefault;
}
