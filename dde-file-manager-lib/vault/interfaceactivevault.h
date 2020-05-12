#ifndef INTERFACEACTIVEVAULT_H
#define INTERFACEACTIVEVAULT_H

#include <QObject>
#include "vaultglobaldefine.h"

class QDialog;

class InterfaceActiveVault : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceActiveVault(QObject *parent = nullptr);
    ~InterfaceActiveVault();

    /**
     * @brief getActiveVaultWidget 获取激活保险箱页面
     * @return 包厢箱激活页面
     */
    QDialog *getActiveVaultWidget();

    /**
     * @brief getPasswordHint 获得密码提示信息
     * @param passwordHint 返回密码提示
     * @return 是否成功
     */
    bool getPasswordHint(QString &passwordHint);

    /**
     * @brief checkPassword 验证密码是否正确
     * @param password 密码
     * @param cliper 返回密码的密文
     * @return 是否成功
     */
    bool checkPassword(const QString &password, QString &cliper);

    /**
     * @brief checkUserKey 验证密钥是否正确
     * @param userKey 密钥
     * @param cliper 返回密码的密文
     * @return 是否成功
     */
    bool checkUserKey(const QString &userKey, QString &cliper);

    // 获得保险箱状态
    EN_VaultState vaultState();

signals:

public slots:

private:
    QDialog         *m_pWidget;
};

#endif // INTERFACEACTIVEVAULT_H
