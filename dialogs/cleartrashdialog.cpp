#include "cleartrashdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>

ClearTrashDialog::ClearTrashDialog(QWidget *parent):
    DBaseDialog(parent)
{

    QString icon = ":/images/dialogs/images/user-trash-full.png";
    QString message = tr("Are you sure to empty trash?");
    QString tipMessage = tr("This action cannot be restored");
    QStringList buttons, buttonTexts;
    buttons << "Cancel" << "Empty";
    buttonTexts << tr("Cancel") << tr("Empty");
    initUI(icon, message, tipMessage, buttons, buttons);
    moveCenter();
    getButtonsGroup()->button(1)->setFocus();
    setButtonTexts(buttonTexts);
}

void ClearTrashDialog::handleKeyEnter(){
    handleButtonsClicked(1);
}

ClearTrashDialog::~ClearTrashDialog()
{

}

