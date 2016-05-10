#include "dscrollbar.h"
#include <QDebug>
#include <QWheelEvent>
#include <QPalette>
#include "utils/utils.h"
#include <QPainter>


DScrollBar::DScrollBar(QWidget *parent)
    :QScrollBar(parent)
{
    setObjectName("DScrollBar");
    m_timer = new QTimer(this);
    m_opacityTimer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hidden()));
    connect(m_opacityTimer, SIGNAL(timeout()), this, SLOT(opacity()));
    setAutoFillBackground(true);
    //m_timer->start(1000);

    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 4px;}");
}

void DScrollBar::wheelEvent(QWheelEvent *e)
{
    m_timer->start(1000);
    setStyleSheet("QScrollBar#DScrollBar:handle:vertical{\
                  border: 0px;\
                  background: rgba(16,16,16,0.5);\
                  width: 6px;}");
    QScrollBar::wheelEvent(e);
}

void DScrollBar::enterEvent(QEvent *e)
{
    m_timer->stop();
    setStyleSheet("QScrollBar#DScrollBar:handle:vertical{\
                  border: 0px;\
                  background: rgba(16,16,16,0.7);\
                  width: 6px;}");

    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 6px;}");
    QScrollBar::enterEvent(e);
}

void DScrollBar::leaveEvent(QEvent *e)
{
    m_timer->start(1000);
    setStyleSheet("QScrollBar#DScrollBar:handle:vertical{\
                  border: 0px;\
                  background: rgba(16,16,16,0.45);\
                  width: 4px;}");

    setStyleSheet("QScrollBar#DScrollBar:vertical{\
                  border: 0px;\
                  background: transparent;\
                  width: 6px;}");
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
    m_opacityTimer->start(50);
    m_count = 20;
    m_level = m_count;

}

void DScrollBar::opacity()
{
    if(m_count > 0)
    {
        m_count--;
        m_opacityTimer->start(50);
        QString stylesheet = "QScrollBar#DScrollBar:handle:vertical{\
                border: 0px;\
                background: rgba(16,16,16," + QString::number(m_count/m_level/2)+ ");" +
                "width: 4px;}";
        setStyleSheet(stylesheet);
    }
    else
    {
        setStyleSheet("QScrollBar#DScrollBar:vertical{\
                      border: 0px;\
                      background: transparent;\
                      width: 1px;}");
        m_opacityTimer->stop();
    }
}
