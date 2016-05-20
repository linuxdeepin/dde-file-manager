#ifndef DDRAG_H
#define DDRAG_H

#include <QDrag>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QDragMoveEvent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QPainter>
#include <QPixmap>

class PixmapWidget : public QWidget
{
public:
    PixmapWidget();
    void setPixmap(const QPixmap &pixmap);
protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void paintEvent(QPaintEvent *e);
    QPixmap m_pixmap;
};

class DDragWidget : public QDrag
{
    Q_OBJECT
public:
    DDragWidget(QObject * parent);
    ~DDragWidget();
    void startDrag();
    void setPixmap(const QPixmap & pixmap);
    void setHotSpot(const QPoint &hotspot);
    bool eventFilter(QObject *obj, QEvent *e);
private:
    PixmapWidget * m_widget;
    QTimer * m_timer;
    QPoint m_hotspot;
public slots:
    void timerEvent();
};

#endif // DDRAG_H
