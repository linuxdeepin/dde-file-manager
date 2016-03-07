#include "dtabbaritem.h"
#include "dtabbar.h"
#include <QDebug>
#include <QPainter>

DTabBarItem::DTabBarItem(DTabBar * tabBar, const QString & text, int index):QLabel(tabBar)
{
    m_selected = false;
    m_tabBar = tabBar;
    m_index = index;
    setText(text);
    setStyleSheet("font: 13px;min-width: 20px; margin-left: 5px;margin-right: 5px");
    setMargin(0);
    setDefaultColor(Qt::white);
    setSelectedColor(Qt::lightGray);
    setBorderColor(Qt::gray);
}

/**
 * @brief TabBarItem::mouseReleaseEvent
 * @param event
 *
 * Callback function for mouse release event and emit a tabbar clicked signal.
 */
void DTabBarItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit m_tabBar->tabBarClicked(m_index);
}

/**
 * @brief TabBarItem::paintEvent
 * @param event
 *
 * Callback function for paint event.
 */
void DTabBarItem::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    double w = width();
    double h = height();
    QPolygon polygon;
    polygon << QPoint(0, 0);
    polygon << QPoint(w -5, 0);
    polygon << QPoint(w, h/2);
    polygon << QPoint(w -5, h);
    polygon << QPoint(0, h);
    polygon << QPoint(0 + 5, h/2);
    QPen pen(m_borderColor);
    pen.setWidthF(1.2);
    painter.setPen(pen);
    if(m_selected)
        painter.setBrush(m_selectedColor);
    else
        painter.setBrush(m_backgroundColor);
    painter.drawPolygon(polygon);
    painter.end();
    QLabel::paintEvent(event);
}

/**
* @brief TabBarItem::setDefaultColor
* @param color
*
* Set the background color of the tab.
*/
void DTabBarItem::setDefaultColor(QColor color)
{
    m_backgroundColor = color;
}

/**
* @brief TabBarItem::setSelectedColor
* @param color
*
* Set the selected color of the tab.
*/
void DTabBarItem::setSelectedColor(QColor color)
{
    m_selectedColor = color;
}

/**
 * @brief TabBarItem::setBorderColor
 * @param color
 *
 * Set the border color of the tab.
 */
void DTabBarItem::setBorderColor(QColor color)
{
    m_borderColor = color;
}

bool DTabBarItem::isSelected()
{
    return m_selected;
}

void DTabBarItem::setSelected(bool select)
{
    m_selected = select;
}

