#ifndef PROGRESSLINE_H
#define PROGRESSLINE_H

#include <QObject>
#include <QLabel>

class ProgressLine : public QLabel
{
    Q_OBJECT
public:
    explicit ProgressLine(QWidget *parent = 0);

    qreal value() const;
    void setValue(const qreal &value);

    qreal max() const;
    void setMax(const qreal &max);

    qreal min() const;
    void setMin(const qreal &min);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    qreal m_value;
    qreal m_max;
    qreal m_min;
};

#endif // PROGRESSLINE_H
