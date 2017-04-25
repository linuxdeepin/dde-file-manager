#include "button.h"

#define NO_ATTRACT_STYLE "QPushButton { color: white; border-width: 6 6 10 6 }" \
    "QPushButton { border-image: url(':/images/transparent_button_normal.svg') 6 6 10 6 }" \
    "QPushButton:hover { border-image: url(':/images/transparent_button_hover.svg') 6 6 10 6 }" \
    "QPushButton:pressed { border-image: url(':/images/transparent_button_press.svg') 6 6 10 6 }"

#define ATTRACT_STYLE "QPushButton { color: white; border-width: 6 6 10 6 }" \
    "QPushButton { border-image: url(':/images/button_normal.svg') 6 6 10 6 }" \
    "QPushButton:hover { border-image: url(':/images/button_hover.svg') 6 6 10 6 }" \
    "QPushButton:pressed { border-image: url(':/images/button_press.svg') 6 6 10 6 }"

Button::Button(QWidget * parent) :
    QPushButton(parent)
{
    setFlat(true);
    setFixedSize(160, 36);
}

Button::~Button()
{

}

void Button::setAttract(bool attract)
{
    if (attract) {
        setStyleSheet(ATTRACT_STYLE);
    } else {
        setStyleSheet(NO_ATTRACT_STYLE);
    }
}
