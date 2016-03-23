#include "dmenu.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>

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
        border: 1px solid lightgray;\
        padding-top: 5px;\
        padding-left: 0px;\
        padding-right: 0px;\
        padding-bottom: 5px;\
    }\
    QMenu::item{\
        padding: 6px 25px 6px 25px;\
        font: 13px;\
    }\
    QMenu::item:hover{\
      background: #636363;\
      color:white;\
    }\
    QMenu::item:disabled{\
      color: gray;\
    }\
    QMenu::separator{\
        height: 6px;\
        margin-left: 0px;\
        margin-right: 0px;\
        padding-left: -5px;\
        padding-right: -5px;\
    }\
    QMenu::item:selected {\
        background: #646464;\
        color:white;\
    }");
}
