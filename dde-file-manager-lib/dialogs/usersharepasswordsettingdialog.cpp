#include "usersharepasswordsettingdialog.h"
#include "../dbusinterface/usershare_interface.h"
#include "../app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"
#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QVBoxLayout>

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent) : DDialog(parent)
{
    setTitle(tr("Please enter share password"));
    setIcon(QIcon(":/images/dialogs/images/share_password.png"));
    initUI();
}

void UserSharePasswordSettingDialog::initUI()
{
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Confirm");
    addButton(buttonTexts[0], true);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);
    m_passwordEdit = new DPasswordEdit();
    m_passwordEdit->setParent(this);
    m_passwordEdit->setFixedSize(240,24);
    m_passwordEdit->setFocus();
    addContent(m_passwordEdit);
    setContentsMargins(0,0,0,0);
//    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
}

void UserSharePasswordSettingDialog::onButtonClicked(const int &index)
{
    if(index == 1){
        // set usershare password
        QString password = m_passwordEdit->text();
        if(password.isEmpty())
            return;
        userShareManager->setSambaPassword(UserShareManager::getCurrentUserName(), password);
    }
}
