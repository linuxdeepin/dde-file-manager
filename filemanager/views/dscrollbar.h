#ifndef DSCROLLBAR_H
#define DSCROLLBAR_H

#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class DScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    DScrollBar(QWidget * parent = 0);
    void wheelEvent(QWheelEvent *e);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    QPropertyAnimation * m_animate;
    QTimer* m_timer;
    QGraphicsOpacityEffect * m_opacity;
public slots:
    void hidden();
};

#endif // DSCROLLBAR_H
