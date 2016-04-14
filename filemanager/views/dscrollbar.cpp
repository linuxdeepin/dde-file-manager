#include "dscrollbar.h"
#include <QDebug>
#include <QWheelEvent>
#include <QPalette>
#include "utils/utils.h"


DScrollBar::DScrollBar(QWidget *parent)
    :QScrollBar(parent)
{
    setObjectName("DScrollBar");
    m_opacity = new QGraphicsOpacityEffect;
    setGraphicsEffect(m_opacity);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hidden()));
    m_timer->start(1000);
}

void DScrollBar::wheelEvent(QWheelEvent *e)
{
    m_timer->start(1000);
    m_opacity->setOpacity(1);
    QScrollBar::wheelEvent(e);
}

void DScrollBar::enterEvent(QEvent *e)
{
    m_timer->stop();
    m_opacity->setOpacity(1);
    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 6px;}");
    QScrollBar::enterEvent(e);
}

void DScrollBar::leaveEvent(QEvent *e)
{
    m_timer->start(1000);
    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 4px;}");
    QScrollBar::leaveEvent(e);
}

void DScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    QScrollBar::mouseMoveEvent(e);
}

void DScrollBar::mousePressEvent(QMouseEvent *e)
{
    m_timer->stop();
    QScrollBar::mousePressEvent(e);
}

void DScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    m_timer->start(1000);
    QScrollBar::mouseReleaseEvent(e);
}

void DScrollBar::hidden()
{
    m_timer->stop();
    QPropertyAnimation * anim = new QPropertyAnimation( m_opacity, "opacity" );
    anim->setDuration( 500 );
    anim->setStartValue( 1 );
    anim->setEndValue( 0.0 );
    anim->start(QPropertyAnimation::DeleteWhenStopped);
    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 4px;}");
}
