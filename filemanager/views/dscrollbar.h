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
    void showEvent(QShowEvent *e);
private:

    QPropertyAnimation * m_animate;
    QTimer* m_timer;
    QTimer* m_opacityTimer;
    QGraphicsOpacityEffect * m_opacity;
    int m_count;
    float m_level;
    QString m_handleStyle;
    QString m_hoverHandleStyle;
    QString m_style;
public slots:
    void hidden();
    void opacity();
};

#endif // DSCROLLBAR_H
