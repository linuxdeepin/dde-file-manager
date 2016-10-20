#include "usersharepasswordsettingdialog.h"
#include <QDebug>
#include <QProcess>
#include "../dbusinterface/usershare_interface.h"
#include <QDBusReply>
#include <QVBoxLayout>

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent) : DDialog(parent)
{
    QStringList buttonTexts;
    buttonTexts << tr("Cancle") << tr("Comfirm");
    addButtons(buttonTexts);
    setDefaultButton(1);
    setTitle(tr("Please enter share password"));
    setIcon(QIcon(":/images/dialogs/images/share_password.png"));
    initUI();
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

void UserSharePasswordSettingDialog::initUI()
{
    m_passwordEdit = new DPasswordEdit(this);
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
        QString pwd = m_passwordEdit->text();
        if(pwd.isEmpty())
            return;
        QProcess up;
        up.start("id",QStringList() << "-u" << "-n");
        up.waitForFinished();
        QByteArray data = up.readAll();
        QString userName = data.data();
        // throw out '\n' string
        userName = userName.trimmed();

        UserShareInterface *userShareInterface = new UserShareInterface("com.deepin.filemanager.daemon",
                                                                        "/com/deepin/filemanager/daemon/UserShareManager",
                                                                        QDBusConnection::systemBus(),
                                                                        this);
        QDBusReply<bool> reply = userShareInterface->setUserSharePassword(userName, pwd);
        if(reply.isValid()){
            qDebug() << "set usershare password:" << reply.value();
        }else{
            qDebug() <<"set usershare password:" << reply.error();
        }

        userShareInterface->deleteLater();
    }
}
