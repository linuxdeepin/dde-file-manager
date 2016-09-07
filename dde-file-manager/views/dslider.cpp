#include "dslider.h"
#include <QDebug>
#include <QtCore>
#include <QPropertyAnimation>

DSLiderHandle::DSLiderHandle(QGraphicsItem *parent):
    QGraphicsObject(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void DSLiderHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPixmap pixmap(":/icons/images/icons/slider_handle.png");
    painter->drawPixmap(0,0,SLIDER_HANDLE_SIZE,SLIDER_HANDLE_SIZE,pixmap);
}

QRectF DSLiderHandle::boundingRect() const
{
    return QRectF(0,0,SLIDER_HANDLE_SIZE,SLIDER_HANDLE_SIZE);
}

bool DSLiderHandle::isDragging() const
{
    return m_dragging;
}

void DSLiderHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_dragging = true;
    QGraphicsObject::mousePressEvent(event);
}

void DSLiderHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
     m_dragging = false;
     emit draggingFinished();
     QGraphicsObject::mouseReleaseEvent(event);
}

DSliderGroove::DSliderGroove(QGraphicsItem *parent):QGraphicsObject(parent)
{
//    setPos(SLIDER_HANDLE_SIZE,(scene()->sceneRect().height() - 2)/2);
}

QRectF DSliderGroove::boundingRect() const
{
    return QRectF(0,0,scene()->sceneRect().width() -SLIDER_HANDLE_SIZE ,2);
}

void DSliderGroove::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //background
    QColor color;
    color.setNamedColor("#cccccc");
    QPainterPath path;
    path.addRoundedRect(boundingRect(),1,1);
    painter->fillPath(path,color);

    QPainterPath addPagePath;
    color.setNamedColor("#2ca7f8");
    int addPageWidth = boundingRect().width()*(m_value - m_minValue) / (m_maxValue - m_minValue);
    addPagePath.addRoundedRect(0,0,addPageWidth,boundingRect().height(),1,1);
    painter->fillPath(addPagePath,color);
}

void DSliderGroove::setValue(const qreal &value)
{
    m_value = value;
    update();
}

void DSliderGroove::setMaxValue(const qreal &value)
{
    m_maxValue = value;
    update();
}

void DSliderGroove::setMinValue(const qreal &value)
{
    m_minValue = value;
    update();
}

DSlider::DSlider(QWidget *parent) : QGraphicsView(parent)
{
    initUI();
    initConnections();
}

void DSlider::initUI()
{
    setMouseTracking(true);
    setFixedHeight(22);
    setContentsMargins(0,0,0,0);
    m_scene = new QGraphicsScene(this);
    m_handle = new DSLiderHandle;
    m_groove = new DSliderGroove;
    QPainterPath path;
    path.addRect(m_scene->sceneRect());
    m_scene->setSelectionArea(path,Qt::IntersectsItemShape,QTransform());
    m_scene->setSceneRect(0,0,width(),height());

    m_scene->addItem(m_groove);
    m_scene->addItem(m_handle);
    setScene(m_scene);

    m_groove->setPos(SLIDER_HANDLE_SIZE/2,(height() -2)/2);
}

void DSlider::initConnections()
{
    connect(m_handle, &DSLiderHandle::xChanged, this, [=]{
        qreal value = m_minValue + (m_maxValue - m_minValue)*
                (m_handle->pos().x()/(width() - SLIDER_HANDLE_SIZE));
        m_groove->setValue(value);
    });
}

void DSlider::setValue(const qreal value)
{
    m_value = value;
    qreal x = (width() - SLIDER_HANDLE_SIZE)*(value - m_minValue)/(m_maxValue - m_minValue);
    QPropertyAnimation *animation = new QPropertyAnimation(m_handle,"x");
    animation->setDuration(150);
    animation->setStartValue(m_handle->x());
    animation->setEndValue(x);
    animation->start();
    connect(animation, &QPropertyAnimation::finished,this,[=]{
        animation->deleteLater();
    });
}

qreal DSlider::value() const
{
    return m_value;
}

qreal DSlider::maxValue() const
{
    return m_maxValue;
}

void DSlider::setMaxValue(const qreal value)
{
    m_maxValue = value;
    m_groove->setMaxValue(value);
}

qreal DSlider::minValue() const
{
    return m_minValue;
}

void DSlider::setMinValue(const qreal value)
{
    m_minValue = value;
    m_groove->setMinValue(value);
}

qreal DSlider::stepSize() const
{
    return m_stepSize;
}

void DSlider::setStepSize(const qreal stepSize)
{
    m_stepSize = stepSize;
}

QSize DSlider::handlePadSize()
{
    return m_handle->boundingRect().size().toSize();
}

void DSlider::resizeEvent(QResizeEvent *event)
{
    m_scene->setSceneRect(0,0,width(),height());
    QGraphicsView::resizeEvent(event);
}

void DSlider::mouseMoveEvent(QMouseEvent *event)
{
    qreal gap = event->pos().x() - m_lastMousePos.x();
    m_lastMousePos = event->pos();

    if((m_handle->x() == 0 && gap<0) ||
            (m_handle->x() == width() - SLIDER_HANDLE_SIZE && gap>0)||
            event->pos().x()<0 ||
            event->pos().x()>width()){
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if(m_handle->isDragging()){
        m_handle->setX(event->x() - SLIDER_HANDLE_SIZE/2);

        if( m_handle->x()<0)
            m_handle->setX(0);
        if(m_handle->x()>width() - SLIDER_HANDLE_SIZE)
            m_handle->setX(width() - SLIDER_HANDLE_SIZE);

        qreal value = m_minValue + (m_maxValue - m_minValue)*
                (m_handle->pos().x()/(width() - SLIDER_HANDLE_SIZE));
        m_groove->setValue(value);

        if(gap>0){
            m_counter = qFloor(value/m_stepSize);
            if(value -m_counter*m_stepSize>m_stepSize*3/5)
                m_counter++;
        }
        else{
            m_counter = qCeil(value/m_stepSize);
            if(m_counter*m_stepSize-value>m_stepSize*3/5)
                m_counter --;
        }

        emit valueChanged( m_stepSize*m_counter);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void DSlider::mouseReleaseEvent(QMouseEvent *event)
{
    qreal target = m_counter*m_stepSize;
    qreal x = (width() - SLIDER_HANDLE_SIZE)*(target - m_minValue)/(m_maxValue - m_minValue);
    QPropertyAnimation *animation = new QPropertyAnimation(m_handle,"x");
    animation->setDuration(150);
    animation->setStartValue(m_handle->x());
    animation->setEndValue(x);
    animation->start();
    connect(animation,&QPropertyAnimation::finished,this,[=]{animation->deleteLater();});
    QGraphicsView::mouseReleaseEvent(event);
}


