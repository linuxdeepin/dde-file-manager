#include "messagewrongdialog.h"


MessageWrongDialog::MessageWrongDialog(const QString &msg, QWidget *parent):
    DBaseDialog(parent)
{
    QString icon = ":/images/dialogs/images/dialog-warning.svg";
    QString message =  msg + " is not a valid url.";
    QString tipMessage = tr("");
    QStringList buttons, buttonTexts;
    buttons << "Confirm";
    buttonTexts << tr("Confirm");
    initUI(icon, message, tipMessage, buttons, buttons);
    moveCenter();
    setButtonTexts(buttonTexts);
}

MessageWrongDialog::~MessageWrongDialog()
{

}

