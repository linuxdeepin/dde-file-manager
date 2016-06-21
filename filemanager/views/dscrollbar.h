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

public slots:
    void hidden();
    void opacity();

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;

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
};

#endif // DSCROLLBAR_H
