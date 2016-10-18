#include "usersharepasswordsettingdialog.h"
#include <QDebug>
#include <QProcess>
#include "../dbusinterface/usershare_interface.h"
#include <QDBusReply>

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent) : DDialog(parent)
{
    QStringList buttonTexts;
    buttonTexts << tr("Cancle") << tr("Comfirm");
    addButtons(buttonTexts);
    setDefaultButton(0);
    setTitle(tr("Please enter share password"));
    setIcon(QIcon(":/icons/images/icons/share_password.png"));
    initUI();
    initConnections();
}

void UserSharePasswordSettingDialog::initUI()
{
    m_passwordEdit = new DPasswordEdit(this);
    addContent(m_passwordEdit);
}

void UserSharePasswordSettingDialog::initConnections()
{
    connect(this, &UserSharePasswordSettingDialog::buttonClicked, this, &UserSharePasswordSettingDialog::onButtonClicked);
}

void UserSharePasswordSettingDialog::onButtonClicked(const int &index, const QString& text)
{
    Q_UNUSED(text)
    if(index == 1){
        // set usershare password
        QString pwd = m_passwordEdit->text();
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
