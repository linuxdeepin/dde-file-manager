#ifndef DMOVABLEMAINWINDOW_H
#define DMOVABLEMAINWINDOW_H

#include <QMainWindow>
class QMouseEvent;

class DMovableMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DMovableMainWindow(QWidget *parent = 0);
    ~DMovableMainWindow();

    QRect getAvailableRect(); // return geometry() - boderSize;
    int getBorderCornerSize();

signals:
    void startMoving();

public slots:
    void setBorderCornerSize(const int borderCornerSize);
    void setDragMovableHeight(const int height);
    void setDragMovableRect(const QRect& rect);
    void moveCenter();
    void moveTopRight();
    void moveCenterByRect(QRect rect);
    void moveTopRightByRect(QRect rect);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    int m_borderCornerSize = 8;
    int m_dragMovableHeight = 0;
    QRect m_dragMovableRect;
    QPoint m_dragPosition;
    QPoint m_pressedPosition;
};

#endif // DMOVABLEMAINWINDOW_H
