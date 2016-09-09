#include "dmovablemainwindow.h"
#include "xutil.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>

DMovableMainWindow::DMovableMainWindow(QWidget *parent) : QMainWindow(parent)
{

}

DMovableMainWindow::~DMovableMainWindow()
{

}

QRect DMovableMainWindow::getAvailableRect()
{
    return QRect(m_borderCornerSize, m_borderCornerSize, width() - 2 * m_borderCornerSize, height() - 2 * m_borderCornerSize);
}

int DMovableMainWindow::getBorderCornerSize()
{
    return m_borderCornerSize;
}

void DMovableMainWindow::setBorderCornerSize(const int borderCornerSize)
{
    m_borderCornerSize = borderCornerSize;
}

void DMovableMainWindow::setDragMovableHeight(const int height){
    QRect dragMovableRect(m_borderCornerSize, m_borderCornerSize, width() - 2 * m_borderCornerSize, height - m_borderCornerSize);
    setDragMovableRect(dragMovableRect);
}

void DMovableMainWindow::setDragMovableRect(const QRect &rect)
{
    m_dragMovableRect = rect;
    m_dragMovableHeight = rect.height();
}

void DMovableMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isActiveWindow()){
        if (m_dragMovableRect.contains(event->pos())){
            emit mouseMoved();
            setFocus();
        }
    }
    QMainWindow::mouseMoveEvent(event);
}



