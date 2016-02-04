#include "confirmdeletedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>

ConfirmDeleteDialog::ConfirmDeleteDialog(QWidget *parent) : DBaseDialog(parent)
{
    QString icon = ":/images/dialogs/images/user-trash-full.png";
    QString message = "Are you sure to delete this item?";
    QString tipMessage = tr("This action cannot be restored");
    QStringList buttons, buttonTexts;
    buttons << "Cancel" << "Delete";
    buttonTexts << tr("Cancel") << tr("Delete");
    initUI(icon, message, tipMessage, buttons, buttons);
    moveCenter();
    getButtonsGroup()->button(1)->setFocus();
    setButtonTexts(buttonTexts);
}

void ConfirmDeleteDialog::handleKeyEnter(){
    handleButtonsClicked(1);
}

ConfirmDeleteDialog::~ConfirmDeleteDialog()
{

}

