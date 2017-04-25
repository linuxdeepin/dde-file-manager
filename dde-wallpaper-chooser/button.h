#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>

class Button : public QPushButton
{
    Q_OBJECT
public:
    Button(QWidget * parent = 0);
    ~Button();

    void setAttract(bool);
};

#endif // BUTTON_H
