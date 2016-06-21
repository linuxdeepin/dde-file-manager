#include "dscrollbar.h"
#include "utils/utils.h"

#include <QDebug>
#include <QWheelEvent>
#include <QPalette>

DScrollBar::DScrollBar(QWidget *parent)
    :QScrollBar(parent)
{
    setObjectName("DScrollBar");
    m_timer = new QTimer(this);
    m_opacityTimer = new QTimer(this);

    m_timer->setInterval(1000);
    m_timer->setSingleShot(true);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(hidden()));
    connect(m_opacityTimer, SIGNAL(timeout()), this, SLOT(opacity()));
    connect(this, &DScrollBar::valueChanged, this, [this] {
        m_timer->start();

        setStyleSheet(m_style + m_handleStyle);
    });

    m_style = "QScrollBar#DScrollBar:vertical{\
            border-radius: 2px;\
            background: transparent;\
            width: 8px;\
            margin: 4px 2px 4px 2px;\
       }\
       QScrollBar#DScrollBar::handle:vertical{\
            min-height: 20px;\
       }\
       QScrollBar#DScrollBar::handle:vertical:hover{\
            border-radius: 2px; \
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
               border-radius: 2px ;\
               background-color: rgba(16,16,16,0.5);\
               min-height: 20px;\
          }";
    m_hoverHandleStyle = "QScrollBar#DScrollBar::handle:vertical{\
               border-radius: 2px;\
               background-color: rgba(16,16,16,0.7);\
               min-height: 20px;\
          }";

    setStyleSheet(m_style + m_handleStyle);
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
    setStyleSheet(m_style + m_hoverHandleStyle);
    QScrollBar::leaveEvent(e);
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
    if (m_count > 0) {
        m_count--;
        m_opacityTimer->start(50);
        QString stylesheet = "QScrollBar#DScrollBar:handle:vertical{\
                border-radius: 2px;\
                background: rgba(16,16,16," + QString::number(m_count/m_level/2)+ ");" +
                "}";
        setStyleSheet(m_style + stylesheet);
    } else {
        m_opacityTimer->stop();
    }
}
