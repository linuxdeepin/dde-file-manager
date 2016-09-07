#ifndef DSLIDER_H
#define DSLIDER_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#define SLIDER_HANDLE_SIZE 22
class DSLiderHandle: public QGraphicsObject{
    Q_OBJECT
public:
    explicit DSLiderHandle(QGraphicsItem *parent = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    bool isDragging() const;
signals:
    void draggingFinished();
    void handlePosChanged(const QPoint pos);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
private:
    bool m_dragging = false;
    QPointF m_pressPos;
};

class DSliderGroove: public QGraphicsObject
{
public:
    explicit DSliderGroove(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setValue(const qreal &value);
    void setMaxValue(const qreal &value);
    void setMinValue(const qreal &value);

private:
    qreal m_value;
    qreal m_maxValue;
    qreal m_minValue;
};

class DSlider : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DSlider(QWidget *parent = 0);
    void initUI();
    void initConnections();
    void setValue(const qreal value);
    qreal value() const;
    qreal maxValue() const;
    void setMaxValue(const qreal value);
    qreal minValue() const;
    void setMinValue(const qreal value);
    qreal stepSize() const;
    void setStepSize(const qreal stepSize);
    QSize handlePadSize();

signals:
    void valueChanged(const qreal value);

public slots:

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    qreal m_value;
    qreal m_maxValue;
    qreal m_minValue;
    qreal m_stepSize;
    DSLiderHandle *m_handle;
    DSliderGroove *m_groove;
    QGraphicsScene *m_scene;
    QPoint m_lastMousePos;
    int m_counter;
};

#endif // DSLIDER_H
