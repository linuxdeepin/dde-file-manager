#include "diskspaceoutofusedtipdialog.h"
#include <DTitlebar>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QFrame>

DiskSpaceOutOfUsedTipDialog::DiskSpaceOutOfUsedTipDialog(QWidget *parent) : DDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|
                   Qt::WindowCloseButtonHint|
                   Qt::Dialog);
    setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));

    setTitle(tr("Target disk doesn't have enough space, unable to copy!"));
    setFixedHeight(125);
    addButton(tr("OK"));
}
