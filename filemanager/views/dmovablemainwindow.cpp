#include "dmovablemainwindow.h"
#include "utils/xutil.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>

DMovableMainWindow::DMovableMainWindow(QWidget *parent) : QMainWindow(parent)
{
    setAttribute(Qt::WA_Hover, true);
    installEventFilter(this);
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

void DMovableMainWindow::moveCenter(){
    QRect qr = frameGeometry();
    QPoint cp;
    cp = qApp->desktop()->availableGeometry().center();
    qr.moveCenter(cp);
    move(qr.topLeft());
}

void DMovableMainWindow::moveTopRight(){
    QRect pRect;
    pRect = qApp->desktop()->availableGeometry();
    int x = pRect.width() - width();
    move(QPoint(x, 0));
}

void DMovableMainWindow::moveTopRightByRect(QRect rect){
    int x = rect.x() + rect.width() - width();
    move(QPoint(x, 0));
}

void DMovableMainWindow::moveCenterByRect(QRect rect){
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}

void DMovableMainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        m_pressedPosition = event->pos();
        if (m_dragMovableRect.contains(event->pos())){
            setCursor(QCursor(Qt::SizeAllCursor));
        }
    }else{
        m_pressedPosition = QPoint(0, 0);
    }
    QMainWindow::mousePressEvent(event);
}

void DMovableMainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        if (m_dragMovableRect.contains(event->pos())){
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
    QMainWindow::mouseReleaseEvent(event);
}

void DMovableMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isActiveWindow()){
        if (m_dragMovableRect.contains(event->pos())){
            QtX11::utils::MoveWindow(this, event, m_dragMovableRect);
        }else{
            QtX11::utils::ResizeWindow(this, event, m_borderCornerSize);
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

void DMovableMainWindow::resizeEvent(QResizeEvent *event){
    setDragMovableRect(QRect(m_borderCornerSize, m_borderCornerSize,  width() - 2 * m_borderCornerSize, m_dragMovableRect.height()));
    QMainWindow::resizeEvent(event);
}

bool DMovableMainWindow::eventFilter(QObject *obj, QEvent *event)
{
//    if (event->type() == QEvent::HoverMove){
//        QRect availableRect = getAvailableRect();
//        QPoint currentPos = mapFromGlobal(QCursor::pos());
//        if (availableRect.contains(currentPos)){
//            QtX11::utils::setNormalCursor(this);
//        }else{
//            QtX11::utils::UpdateCursorShape(this, currentPos.x(), currentPos.y(), m_borderCornerSize);
//        }
//    }else if (event->type() == QEvent::HoverLeave){
//        QtX11::utils::setNormalCursor(this);
//    }
    return QMainWindow::eventFilter(obj, event);
}
