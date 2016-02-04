#include "dicontextbutton.h"

DIconTextButton::DIconTextButton(const QIcon &icon, const QString &text, QWidget *parent):
    QPushButton(icon, text, parent)
{
    setIconSize(QSize(20, 20));
    setObjectName("DIconTextButton");
}

DIconTextButton::~DIconTextButton()
{

}
