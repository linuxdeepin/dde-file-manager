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
    //setAutoFillBackground(true);
    //m_timer->start(1000);
    m_style = "QScrollBar#DScrollBar:vertical{\
            border: 0px;\
            background: transparent;\
            width: 6px;\
       }\
       QScrollBar#DScrollBar::handle:vertical:hover{\
            background: rgba(16,16,16,0.7);\
       }\
       QScrollBar#DScrollBar::add-line:vertical {\
             border: none;\
             background: none;\
       }\
       QScrollBar#DScrollBar::sub-line:vertical {\
             border: none;\
             background: none;\
       }\
       QScrollBar#DScrollBar::up-arrow:vertical, QScrollBar#DScrollBar::down-arrow:vertical {\
            width: 0px;\
            height: 0px;\
            background: transparent;\
       }";
       m_handleStyle = "QScrollBar#DScrollBar::handle:vertical{\
               border: 0px;\
               background-color: rgba(16,16,16,0);\
               width: 6px;\
          }";
       m_hoverHandleStyle = "QScrollBar#DScrollBar::handle:vertical{\
               border: 0px;\
               background-color: rgba(16,16,16,0.7);\
               width: 8px;\
          }";
}

void DScrollBar::wheelEvent(QWheelEvent *e)
{
    m_timer->start(1000);
    setStyleSheet(m_style + m_hoverHandleStyle);
    QScrollBar::wheelEvent(e);
}

void DScrollBar::enterEvent(QEvent *e)
{
    m_timer->stop();
    setStyleSheet(m_style + m_hoverHandleStyle);
    QScrollBar::enterEvent(e);
}

void DScrollBar::leaveEvent(QEvent *e)
{
    m_timer->start(1000);
    setStyleSheet(m_style + m_handleStyle);
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

void DScrollBar::showEvent(QShowEvent *e)
{
    setStyleSheet(m_style + m_handleStyle);
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
        setStyleSheet(m_style + stylesheet);
    }
    else
    {
        setStyleSheet(m_style + m_handleStyle);
        m_opacityTimer->stop();
    }
}
