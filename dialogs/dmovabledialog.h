#ifndef DMOVABLEDIALOG_H
#define DMOVABLEDIALOG_H


#include <QDialog>
#include <QPoint>
class QMouseEvent;
class QPushButton;
class QResizeEvent;

class DMovabelDialog:public QDialog
{
    Q_OBJECT
public:
    DMovabelDialog(QWidget *parent = 0);
    ~DMovabelDialog();

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

#endif // DMOVABLEDIALOG_H
