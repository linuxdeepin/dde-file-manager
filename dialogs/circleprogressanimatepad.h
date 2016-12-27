#ifndef CIRCLEPROGRESSANIMATEPAD_H
#define CIRCLEPROGRESSANIMATEPAD_H

#include <QLabel>
#include <QTimer>
#include <QColor>

class CircleProgressAnimatePad : public QLabel
{
    Q_OBJECT
public:
    explicit CircleProgressAnimatePad(QWidget *parent = 0);
    void initConnections();

    int lineWidth() const;
    void setLineWidth(int lineWidth);

    int currentValue() const;
    void setCurrentValue(int currentValue);

    int minmumValue() const;
    void setMinmumValue(int minmumValue);

    int maximumValue() const;
    void setMaximumValue(int maximumValue);

    int angle() const;
    void setAngle(int angle);

    QColor chunkColor() const;
    void setChunkColor(const QColor &chunkColor);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &backgroundColor);

    int fontSize() const;
    void setFontSize(int fontSize);

signals:

public slots:
    void startAnimation();
    void stopAnimation();
    void stepAngle();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    int m_lineWidth = 3;
    int m_currentValue = 10;
    int m_minmumValue = 0;
    int m_maximumValue = 100;
    int m_angle = 0;
    int m_fontSize = 12;

    QColor m_chunkColor = Qt::cyan;
    QColor m_backgroundColor = Qt::darkCyan;

    QTimer* m_timer;
    bool isAnimateStarted = false;
};

#endif // CIRCLEPROGRESSANIMATEPAD_H
