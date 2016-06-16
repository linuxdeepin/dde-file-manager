#ifndef DMOVEABLEWIDGET_H
#define DMOVEABLEWIDGET_H

#include <QWidget>
#include <QPoint>
class QMouseEvent;
class QPushButton;
class QResizeEvent;

class DMoveableWidget:public QWidget
{
    Q_OBJECT
public:
    DMoveableWidget(QWidget *parent = 0);
    ~DMoveableWidget();

public slots:
    void setMovableHeight(int height);
    void moveCenter();
    void moveTopRight();
    void moveCenterByRect(QRect rect);
    void moveTopRightByRect(QRect rect);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    QPoint m_dragPosition;
    int m_movableHeight = 30;
};

#endif // DMOVEABLEWIDGET_H
