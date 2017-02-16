#include "movetotrashconflictdialog.h"
#include <QLabel>
#include <QFont>

MoveToTrashConflictDialog::MoveToTrashConflictDialog(QWidget *parent, const DUrlList &urls) :
    DDialog(parent)
{

    QString title;
    QString tip;

    if(urls.size() == 1){
        title = tr("This file is too large to put into trash");
        tip = tr("Are you sure to completely delete %1").arg(urls.first().fileName());
    }
    else{
        title = tr("Files are too large to put into trash");
        tip = tr("Are you sure to completely delete %1 files").arg(QString::number(urls.size()));
    }

    setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));
    setTitle(title);

    QLabel* label = new QLabel(this);
//    label->setWordWrap(true);

    QFont font;
    font.setPixelSize(12);
    label->setFont(font);

    QFontMetrics fm(font);
    QString elidedTipStr = fm.elidedText(tip,Qt::ElideRight,240);
    elidedTipStr += "?";
    label->setText(elidedTipStr);

    addContent(label);

    addButton(tr("Cancel"), false);
    addButton(tr("Delete"), true, DDialog::ButtonWarning);
}
