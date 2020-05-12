#include "interfaceactivevault.h"
#include "activevault/widgetmanager.h"
#include "operatorcenter.h"

InterfaceActiveVault::InterfaceActiveVault(QObject *parent)
    : QObject(parent)
    , m_pWidget(nullptr)
{
    m_pWidget = new WidgetManager();
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

EN_VaultState InterfaceActiveVault::vaultState()
{
    return OperatorCenter::getInstance().vaultState();
}
