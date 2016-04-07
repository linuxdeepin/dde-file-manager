#include "dbookmarkitem.h"
#include "dbookmarkitemgroup.h"
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include "dfilemenu.h"
#include "filemenumanager.h"

DBookmarkItem::DBookmarkItem()
{
    m_hovered = false;
    m_pressed = false;
    m_backgroundEnabled = false;
    m_isDraggable = false;
    m_adjust = 0;
    m_xPos = 0;
    m_yPos = 0;
    m_xOffsetImage = 0;
    m_yOffsetImage = 0;
    m_isMenuOpened = false;
    m_checked = false;
    m_checkable = true;
    m_isDefault = false;
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setAcceptHoverEvents(true);
    setReleaseBackgroundColor(QColor(238,232,205,0));
    setPressBackgroundColor(QColor(44,167,248,255));
    setHoverBackgroundColor(Qt::lightGray);
    setPressBackgroundEnable(true);
    setReleaseBackgroundEnable(true);
    setHoverBackgroundEnable(true);
    setTextColor(Qt::black);
    setAcceptDrops(true);
}

QRectF DBookmarkItem::boundingRect() const
{
    return QRectF(m_x_axis - m_adjust,
                  m_y_axis - m_adjust,
                  m_width + m_adjust,
                  m_height + m_adjust);
}

void DBookmarkItem::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    double w = m_width;
//    double h = m_height;
    QColor textColor;
    double leftPadding = 18;
//    double topPadding = 4;
    if(m_pressed || (m_checked && m_checkable))
    {
        if(m_pressBackgroundEnabled)
        {
            painter->setPen(QColor(0,0,0,0));
            painter->setBrush(m_pressBackgroundColor);
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
            textColor = Qt::white;
        }
        painter->drawPixmap(-w/2 + leftPadding,
                            -m_releaseImage.height()/2,
                            m_releaseImage.width(),
                            m_releaseImage.height(),
                            m_pressImage);

    }
    else if(m_hovered)
    {
        if(m_hoverBackgroundEnabled)
        {
            painter->setBrush(m_hoverBackgroundColor);
            painter->setPen(QColor(0,0,0,0));
            painter->drawRect(m_x_axis, m_y_axis, m_width, m_height);
        }
        painter->drawPixmap(-w/2 + leftPadding,
                            -m_releaseImage.height()/2,
                            m_releaseImage.width(),
                            m_releaseImage.height(),
                            m_hoverImage);
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
        painter->drawPixmap(-w/2 + leftPadding,
                            -m_releaseImage.height()/2,
                            m_releaseImage.width(),
                            m_releaseImage.height(),
                            m_releaseImage);
        textColor = Qt::black;
    }

    if(!m_textContent.isNull())
    {
        painter->setPen(textColor);
        painter->setFont(m_font);
        QRect rect( -w/2 + 42, m_y_axis + m_height/4, m_width - 25, m_height);
        painter->drawText(rect,Qt::TextWordWrap|Qt::AlignLeft,m_textContent);
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

        QDrag *drag = new QDrag((QObject*)event->widget());
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(m_textContent);
        QStyleOptionGraphicsItem opt;
        drag->setPixmap(toPixmap());
        drag->setMimeData(mimeData);
        QPointF p = event->pos() - boundingRect().topLeft();
        QPoint point(p.x(), p.y());
        drag->setHotSpot(point);
        drag->exec();
        m_pressed = false;
        emit dragFinished(QCursor::pos(), this);
    }
}

QPixmap DBookmarkItem::toPixmap() const
{
    if (!scene()) {
        return QPixmap();
    }

    QRectF r = boundingRect();
    QPixmap pixmap(r.width(), r.height());
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter(&pixmap);
    painter.drawRect(r);
    scene()->render(&painter, QRectF(), sceneBoundingRect());
    painter.end();
    return pixmap;
}

void DBookmarkItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    if(m_group && m_pressed)
    {
        emit m_group->url(m_url);
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
    QGraphicsItem::dragEnterEvent(event);
}

void DBookmarkItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    emit dragLeft();
    QGraphicsItem::dragLeaveEvent(event);
}

void DBookmarkItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    m_hovered = true;
    event->accept();
    qDebug() << "item drag move";
}

void DBookmarkItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug() << event;

    event->acceptProposedAction();
    emit dropped();
}

void DBookmarkItem::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void DBookmarkItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_hovered = true;
    update();
}

void DBookmarkItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    if(!m_isMenuOpened)
    {
        m_hovered = false;
        update();
    }
}

void DBookmarkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    qDebug() <<"menu begin";
    m_isMenuOpened = true;
    DFileMenu *menu;
    if(m_isDefault)
        menu = FileMenuManager::createDefaultBookMarkMenu();
    else
        menu = FileMenuManager::createCustomBookMarkMenu();
    menu->setUrl(m_url);
    menu->deleteLater();
    menu->exec(QCursor::pos());
    qDebug() <<"menu done";
    m_isMenuOpened = false;
    m_hovered = false;
    update();
    event->accept();
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

void DBookmarkItem::setUrl(const QString &url)
{
    m_url = url;
}

QString DBookmarkItem::getUrl()
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
