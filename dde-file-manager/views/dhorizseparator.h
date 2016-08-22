#ifndef DHORIZSEPARATOR_H
#define DHORIZSEPARATOR_H

#include <QWidget>

class DHorizSeparator : public QWidget
{
public:
    DHorizSeparator(QWidget * parent = 0);
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // DHORIZSEPARATOR_H
