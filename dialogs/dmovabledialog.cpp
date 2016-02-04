#include "dmovabledialog.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QPainter>
#include <QWidget>
#include <QDebug>


DMovabelDialog::DMovabelDialog(QWidget *parent):QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
}

void DMovabelDialog::setMovableHeight(int height){
    m_movableHeight = height;
}

void DMovabelDialog::moveCenter(){
    QRect qr = frameGeometry();
    QPoint cp;
    if (parent()){
        cp = static_cast<QWidget*>(parent())->geometry().center();
    }else{
        cp = qApp->desktop()->availableGeometry().center();
    }
    qr.moveCenter(cp);
    move(qr.topLeft());
}

void DMovabelDialog::moveTopRight(){
    QRect pRect;
    if (parent()){
        pRect = static_cast<QWidget*>(parent())->geometry();
    }else{
        pRect = qApp->desktop()->availableGeometry();
    }
    int x = pRect.width() - width();
    move(QPoint(x, 0));
}

void DMovabelDialog::moveTopRightByRect(QRect rect){
    int x = rect.x() + rect.width() - width();
    move(QPoint(x, 0));
}

void DMovabelDialog::moveCenterByRect(QRect rect){
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}

void DMovabelDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() & Qt::LeftButton)
    {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    }
    QDialog::mousePressEvent(event);
}

void DMovabelDialog::mouseReleaseEvent(QMouseEvent *event)
{
    QDialog::mouseReleaseEvent(event);
}

void DMovabelDialog::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos() - m_dragPosition);
    QDialog::mouseMoveEvent(event);
}

void DMovabelDialog::resizeEvent(QResizeEvent *event){
    QDialog::resizeEvent(event);
}

void DMovabelDialog::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.setPen(QPen(QColor(255, 255, 255, 51), 2));
    painter.setBrush(QColor(0, 0 , 0, 204));
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect r(1, 1, width() - 2, height() - 2);
    painter.drawRoundedRect(r, 2, 2, Qt::RelativeSize);
    QDialog::paintEvent(event);
}

DMovabelDialog::~DMovabelDialog()
{

}

