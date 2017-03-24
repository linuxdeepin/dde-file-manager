#include "messagedialog.h"
#include <QIcon>

MessageDialog::MessageDialog(QWidget *parent) : DDialog(parent)
{

}

MessageDialog::MessageDialog(const QString &message, QWidget *parent):
    DDialog(parent)
{
    m_msg = message;
    initUI();
}

void MessageDialog::initUI()
{
    setIcon(QIcon(":/dialog/dialog_warning_64.png"));
    addButton(tr("OK"), true, DDialog::ButtonRecommend);
    setTitle(m_msg);
}

