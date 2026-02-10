// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATORCENTER_H
#define OPERATORCENTER_H

#include "dfmplugin_vault_global.h"

#include "utils/vaultdefine.h"

#include <DSecureString>

#include <QObject>

namespace dfmplugin_vault {
class OperatorCenter : public QObject
{
    Q_OBJECT
public:
    static OperatorCenter *getInstance();
    ~OperatorCenter();

    /*!
     * \brief createDirAndFile 创建保险箱目录及密钥文件
     * \return 是否成功
     */
    Result createDirAndFile();

    /*!
     * \brief savePasswordAndPasswordHint Save password, save Password hint to file
     * \param password 密码
     * \param passwordHint 密码提示
     * \return 是否成功
     */
    Result savePasswordAndPasswordHint(const QString &password, const QString &passwordHint);

    /*!
     * \brief createKey rsa生成密钥对，私钥加密密码，将密文写入文件，将一部分公钥写入文件（另一部分公钥提供给用户）
     * \param password 密码
     * \param bytes 提供给用户的那部分公钥的长度
     * \return 是否成功
     */
    bool createKey(const QString &password, int bytes);

    /*!
     * \brief checkPassword 验证密码是否正确
     * \param password 密码
     * \param cipher 如果密码正确，返回加密后的密文，用于解锁保险箱
     * \return 是否正确
     */
    bool checkPassword(const QString &password, QString &cipher);

    /*!
     * \brief checkUserKey 验证用户密钥是否正确
     * \param userKey 用户密钥
     * \param cipher 如果密钥正确，返回加密后的密文，用于解锁保险箱
     * \return 是否正确
     */
    bool checkUserKey(const QString &userKey, QString &cipher);

    /*!
     * \brief getUserKey 获得用户密钥
     * \return 用户密钥
     */
    QString getUserKey();

    /*!
     * \brief getPasswordHint 获取密码提示信息
     * \param passwordHint 返回提示信息
     * \return 是否成功
     */
    bool getPasswordHint(QString &passwordHint);

    //! 获取盐值及密码密文
    QString getSaltAndPasswordCipher();

    //! 清除盐值及密码密文的缓存
    void clearSaltAndPasswordCipher();

    //! 获得加密文件夹路径
    QString getEncryptDirPath();
    //! 获得解密文件夹路径
    QString getdecryptDirPath();

    //! 获得配置文件路径
    QStringList getConfigFilePath();

    //! 随即生成密码
    QString autoGeneratePassword(int minLength);

    //! 管理员权限认证
    bool getRootPassword();

    //! 执行shell命令并获得shell命令的返回值
    int executionShellCommand(const QString &strCmd, QStringList &lstShellOutput);

    Result savePasswordToKeyring(const QString &password);
    QString passwordFromKeyring();

    void removeVault(const QString &basePath);

Q_SIGNALS:
    void fileRemovedProgress(int value);

private:
    explicit OperatorCenter(QObject *parent = nullptr);
    // 组织保险箱本地文件路径
    QString makeVaultLocalPath(const QString &before = "", const QString &behind = "");
    bool runCmd(const QString &cmd);
    bool executeProcess(const QString &cmd);
    // 保存第二次加密后的密文,并更新保险箱版本信息
    bool secondSaveSaltAndCiphertext(const QString &ciphertext, const QString &salt, const char *vaultVersion);
    bool statisticsFilesInDir(const QString &dirPath, int *filesCount);
    void removeDir(const QString &dirPath, int filesCount, int *removedFileCount, int *removedDirCount);

public:
    /*!
     * \brief createKeyNew rsa生成密钥对，私钥加密密码，将密文写入文件
     * \param in password 密码
     * \return 是否成功
     */
    Result createKeyNew(const QString &password);

    /*!
     * \brief saveKey 保存恢复密钥
     * \param key  恢复密钥（新版本是LUKS恢复密钥，旧版本是RSA用户密钥部分）
     * \param path 保存路径
     * \return
     */
    Result saveKey(QString key, QString path);

    /*!
     * \brief getPubKey 获得密码公钥
     * \return 密码公钥
     */
    QString getPubKey();

    /*!
     * \brief setRecoveryKey 设置恢复密钥（用于保存到文件）
     * \param recoveryKey 恢复密钥
     */
    void setRecoveryKey(const QString &recoveryKey);

    /*!
     * \brief getRecoveryKey 获得恢复密钥
     * \return 恢复密钥
     */
    QString getRecoveryKey();

    /*!
     * \brief setPendingOldPasswordSchemeMigrationPassword 设置待迁移的老密码（用于老密码方案迁移流程）
     * \param password 老密码
     */
    void setPendingOldPasswordSchemeMigrationPassword(const QString &password);

    /*!
     * \brief getPendingOldPasswordSchemeMigrationPassword 获得待迁移的老密码
     * \return 老密码
     */
    QString getPendingOldPasswordSchemeMigrationPassword() const;

    /*!
     * \brief generateRecoveryKeyForNewVault 为新版本保险箱预生成恢复密钥（在创建保险箱之前）
     * \return 生成的恢复密钥
     */
    QString generateRecoveryKeyForNewVault();

    /*!
     * \brief verificationRetrievePassword 验证用户密钥是否正确
     * \param1 in keypath 密钥文件
     * \param2 out password 密码
     * \return 是否正确
     */
    bool verificationRetrievePassword(const QString keypath, QString &password);

    /*!
     * \brief isNewVaultVersion 检测保险箱是否使用新密码管理方案
     * \return true表示新版本（存在LUKS容器），false表示旧版本
     */
    bool isNewVaultVersion() const;

    /*!
     * \brief resetPasswordByOldPassword 通过旧密码重置密码
     * \param oldPassword 旧密码
     * \param newPassword 新密码
     * \param passwordHint 新密码提示（可选，可以为空）
     * \return 是否成功
     */
    bool resetPasswordByOldPassword(const QString &oldPassword, const QString &newPassword, const QString &passwordHint);

    /*!
     * \brief resetPasswordByRecoveryKey 通过恢复密钥重置密码
     * \param recoveryKey 恢复密钥（新版本是字符串，旧版本是密钥文件路径）
     * \param newPassword 新密码
     * \param passwordHint 新密码提示（可选，可以为空）
     * \return 是否成功
     */
    bool resetPasswordByRecoveryKey(const QString &recoveryKey, const QString &newPassword, const QString &passwordHint);

    /*!
     * \brief migrateOldVaultByPassword 通过旧密码将老版本保险箱迁移到新密码管理方案
     * \param oldPassword 旧版本保险箱的用户密码
     * \param newPassword 迁移后用于 LUKS 的新用户密码
     * \param outRecoveryKey 迁移后生成或复用的恢复密钥（用于界面保存到文件）
     * \return 是否成功
     */
    bool migrateOldVaultByPassword(const QString &oldPassword,
                                   const QString &newPassword,
                                   QString &outRecoveryKey);

    /*!
     * \brief migrateOldVaultByRecoveryKey 通过恢复密钥将老版本保险箱迁移到新密码管理方案
     * \param recoveryKey 恢复密钥（旧版本从密钥文件中解析得到的字符串）
     * \param newPassword 迁移后用于 LUKS 的新用户密码
     * \param outRecoveryKey 迁移后生成或复用的恢复密钥（用于界面保存到文件）
     * \return 是否成功
     */
    bool migrateOldVaultByRecoveryKey(const QString &recoveryKey,
                                      const QString &newPassword,
                                      QString &outRecoveryKey);

    /*!
     * \brief upgradeOldVaultByPassword 通过验证密码将老版本保险箱迁移到新密码管理方案（不重置密码）
     * \brief 这是从老密码方案（pbkdf2）迁移到新密码管理方案（LUKS）的特定流程
     * \param oldPassword 老密码（用于验证和作为新 LUKS 容器的用户密码）
     * \param outRecoveryKey 迁移后生成的新恢复密钥（用于界面保存到文件）
     * \return 是否成功
     */
    bool upgradeOldVaultByPassword(const QString &oldPassword, QString &outRecoveryKey);

private:
    Dtk::Core::DSecureString strCryfsPassword;   // cryfs密码
    QString strUserKey;
    QString standOutput;
    QString strPubKey;
    QString strRecoveryKey;   // 恢复密钥（新版本保险箱使用）
    QString pendingOldPasswordSchemeMigrationPassword;   // 待迁移的老密码（用于老密码方案迁移流程）
};
}
#endif   // OPERATORCENTER_H
