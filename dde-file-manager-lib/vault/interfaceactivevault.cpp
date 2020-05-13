#include "interfaceactivevault.h"
#include "views/dfmvaultactiveview.h"
#include "operatorcenter.h"

InterfaceActiveVault::InterfaceActiveVault(QObject *parent)
    : QObject(parent)
    , m_pWidget(nullptr)
{
    m_pWidget = new DFMVaultActiveView();
}

InterfaceActiveVault::~InterfaceActiveVault()
{
    if(m_pWidget){
        delete m_pWidget;
        m_pWidget = nullptr;
    }
}

QDialog *InterfaceActiveVault::getActiveVaultWidget()
{
    return m_pWidget;
}

QString InterfaceActiveVault::getPassword()
{
    return OperatorCenter::getInstance().getSaltAndPasswordClipher();
}

bool InterfaceActiveVault::getPasswordHint(QString &passwordHint)
{
    return OperatorCenter::getInstance().getPasswordHint(passwordHint);
}

bool InterfaceActiveVault::checkPassword(const QString &password, QString &cliper)
{
    return OperatorCenter::getInstance().checkPassword(password, cliper);
}

bool InterfaceActiveVault::checkUserKey(const QString &userKey, QString &cliper)
{
    return OperatorCenter::getInstance().checkUserKey(userKey, cliper);
}

QString InterfaceActiveVault::getEncryptDir()
{
    return OperatorCenter::getInstance().getEncryptDirPath();
}

QString InterfaceActiveVault::getDecryptDir()
{
    return OperatorCenter::getInstance().getdecryptDirPath();
}

QStringList InterfaceActiveVault::getConfigFilePath()
{
    return OperatorCenter::getInstance().getConfigFilePath();
}

EN_VaultState InterfaceActiveVault::vaultState()
{
    return OperatorCenter::getInstance().vaultState();
}
