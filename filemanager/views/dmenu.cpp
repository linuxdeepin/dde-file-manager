#include "dmenu.h"
#include <QPainter>

DMenu::DMenu(QWidget *parent)
    :QMenu(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("\
    QMenu {\
        background-color: white;\
        color: black;\
        border-radius: 4px;\
        border-style: inset; \
        border: 1px solid lightGray;\
        padding-top: 10px;\
        padding-left: 0px;\
        padding-right: 0px;\
        padding-bottom: 10px;\
    }\
    QMenu::item{\
        padding-top: 4px;\
        padding-left: 25px;\
        padding-right: 25px;\
        padding-bottom: 4px;\
        font: 13px;\
    }\
    QMenu::item:hover{\
      background: black;\
    }\
    QMenu::separator{\
        padding-left: -5px;\
        padding-right: -5px;\
    }\
    QMenu::item:selected {\
        background: #646464;\
    }");
}
