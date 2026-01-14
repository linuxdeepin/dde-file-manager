// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operatorcenter.h"
#include "vaultconfig.h"
#include "passwordmanager.h"
#include "masterkeymanager.h"
#include "utils/operator/pbkdf2.h"
#include "utils/operator/rsam.h"

#include <dfm-io/dfmio_utils.h>

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTime>
#include <QtGlobal>
#include <QProcess>
#include <QtConcurrent>
#include <QRegularExpression>

#undef signals
extern "C" {
#include <libsecret/secret.h>
}
#define signals public

using namespace dfmplugin_vault;

OperatorCenter::OperatorCenter(QObject *parent)
    : QObject(parent), strCryfsPassword(""), strUserKey(""), standOutput(""), strRecoveryKey(""), pendingOldPasswordSchemeMigrationPassword("")
{
    fmDebug() << "Vault: OperatorCenter initialized";
}

QString OperatorCenter::makeVaultLocalPath(const QString &before, const QString &behind)
{
    return DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(),
                                          before.toStdString().c_str(),
                                          behind.toStdString().c_str(), nullptr);
}

bool OperatorCenter::runCmd(const QString &cmd)
{
    fmDebug() << "Vault: Executing command:" << cmd;
    QProcess process;
    int mescs = 10000;
    if (cmd.startsWith(kRootProxy)) {
        mescs = -1;
        fmDebug() << "Vault: Using root proxy, no timeout";
    }

    process.start(cmd);
    bool res = process.waitForFinished(mescs);
    standOutput = process.readAllStandardOutput();
    int exitCode = process.exitCode();

    if (cmd.startsWith(kRootProxy) && (exitCode == 127 || exitCode == 126)) {
        fmWarning() << "Vault: Run \'" << cmd << "\' fauled: Password Error! " << QString::number(exitCode);
        return false;
    }

    if (res == false)
        fmWarning() << "Vault: Run \'" + cmd + "\' failed!";

    return res;
}

bool OperatorCenter::executeProcess(const QString &cmd)
{
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd);
    }

    runCmd("id -un");
    if (standOutput.trimmed() == "root") {
        fmDebug() << "Vault: Already running as root, executing directly";
        return runCmd(cmd);
    }

    QString newCmd = QString(kRootProxy) + " \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd);
}

bool OperatorCenter::secondSaveSaltAndCiphertext(const QString &ciphertext, const QString &salt, const char *vaultVersion)
{
    fmDebug() << "Vault: Saving second salt and ciphertext, version:" << vaultVersion;

    // 密文
    QString strCiphertext = pbkdf2::pbkdf2EncrypyPassword(ciphertext, salt, kIterationTwo, kPasswordCipherLength);
    if (strCiphertext.isEmpty()) {
        fmWarning() << "Vault: Failed to encrypt password with PBKDF2";
        return false;
    }

    // 写入文件
    QString strSaltAndCiphertext = salt + strCiphertext;
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyCipher, QVariant(strSaltAndCiphertext));
    // 更新保险箱版本信息
    config.set(kConfigNodeName, kConfigKeyVersion, QVariant(vaultVersion));

    fmDebug() << "Vault: Second salt and ciphertext saved successfully";
    return true;
}

bool OperatorCenter::statisticsFilesInDir(const QString &dirPath, int *filesCount)
{
    fmDebug() << "Vault: Statistics files in vault directory";

    QDir dir(dirPath);
    if (!dir.exists()) {
        fmWarning() << "Vault: Vault directory does not exist";
        return false;
    }

    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);
    int count = list.size();
    for (int i = 0; i < count; ++i) {
        (*filesCount)++;
        if (list.at(i).isDir()) {
            statisticsFilesInDir(list.at(i).filePath(), filesCount);
        }
    }

    fmDebug() << "Vault: Total files count:" << *filesCount;
    return true;
}

void OperatorCenter::removeDir(const QString &dirPath, int filesCount, int *removedFileCount, int *removedDirCount)
{
    fmDebug() << "Vault: Removing vault directory, total files:" << filesCount;

    QDir dir(dirPath);
    if (!dir.exists() || filesCount < 1) {
        fmDebug() << "Vault: Directory does not exist or files count is invalid";
        return;
    }

    dir.setSorting(QDir::DirsFirst);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);
    int count = infoList.size();
    fmDebug() << "Vault: Processing" << count << "items in directory";

    for (int i = 0; i < count; ++i) {
        if (infoList.at(i).isDir()) {
            removeDir(infoList.at(i).absoluteFilePath(), filesCount, removedFileCount, removedDirCount);
        } else if (infoList.at(i).isFile()) {
            QFile file(infoList.at(i).absoluteFilePath());
            file.remove();
            (*removedFileCount)++;
            int value = static_cast<int>(100 * static_cast<float>(*removedFileCount + *removedDirCount) / static_cast<float>(filesCount));
            emit fileRemovedProgress(value);
        }
    }

    QDir tempDir;
    tempDir.rmdir(dirPath);
    (*removedDirCount)++;
    int value = static_cast<int>(100 * static_cast<float>(*removedFileCount + *removedDirCount) / static_cast<float>(filesCount));
    emit fileRemovedProgress(value);
}

Result OperatorCenter::createKeyNew(const QString &password)
{
    fmDebug() << "Vault: Creating new key";

    strPubKey.clear();
    QString strPriKey("");
    rsam::createPublicAndPrivateKey(strPubKey, strPriKey);
    fmDebug() << "Vault: RSA key pair created";

    // 私钥加密
    QString strCipher = rsam::privateKeyEncrypt(password, strPriKey);
    fmDebug() << "Vault: Password encrypted with private key";

    // 验证公钥长度
    if (strPubKey.length() < 2 * kUserKeyInterceptIndex + 32) {
        fmCritical("Vault: USER_KEY_LENGTH is too long!");
        strPubKey.clear();
        return { false, tr("Failed to create public key: The key length is too long!") };
    }

    // 保存密文
    QString strCipherFilePath = makeVaultLocalPath(kRSACiphertextFileName);
    QFile cipherFile(strCipherFilePath);
    if (!cipherFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical("Vault: open rsa cipher file failed!");
        return { false, tr("Failed to save rsa ciphertext file: %1").arg(strerror(errno)) };
    }
    QTextStream out2(&cipherFile);
    out2 << strCipher;
    cipherFile.close();
    fmDebug() << "Vault: RSA ciphertext saved to:" << strCipherFilePath;

    return { true };
}

Result OperatorCenter::saveKey(QString key, QString path)
{
    fmDebug() << "Vault: Saving key to path:" << path;

    if (key.isEmpty()) {
        fmWarning() << "Vault: Key is empty";
        return { false, tr("Failed to save public key file: The public key is empty.") };
    }

    // 保存部分公钥
    QString keyFilePath = path;
    QFile keyFile(keyFilePath);
    if (!keyFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open public key file failure!";
        return { false, tr("Failed to save public key file: %1").arg(strerror(errno)) };
    }
    keyFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    QTextStream out(&keyFile);
    out << key;
    keyFile.close();
    fmDebug() << "Vault: Public key saved successfully";
    return { true };
}

QString OperatorCenter::getPubKey()
{
    return strPubKey;
}

void OperatorCenter::setRecoveryKey(const QString &recoveryKey)
{
    strRecoveryKey = recoveryKey;
}

QString OperatorCenter::getRecoveryKey()
{
    return strRecoveryKey;
}

void OperatorCenter::setPendingOldPasswordSchemeMigrationPassword(const QString &password)
{
    pendingOldPasswordSchemeMigrationPassword = password;
}

QString OperatorCenter::getPendingOldPasswordSchemeMigrationPassword() const
{
    return pendingOldPasswordSchemeMigrationPassword;
}

QString OperatorCenter::generateRecoveryKeyForNewVault()
{
    char recoveryKeyBuf[33];
    int ret = PasswordManager::generateSecureRecoveryKey(recoveryKeyBuf, sizeof(recoveryKeyBuf));
    if (ret != 0) {
        fmWarning() << "Vault: Failed to generate recovery key for new vault";
        return QString();
    }

    QString recoveryKey = QString::fromUtf8(recoveryKeyBuf, 32);
    setRecoveryKey(recoveryKey);
    return recoveryKey;
}

bool OperatorCenter::verificationRetrievePassword(const QString keypath, QString &password)
{
    bool isNewVersion = isNewVaultVersion();

    if (isNewVersion) {
        QFile keyFile(keypath);
        if (!keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fmCritical() << "Vault: Cannot open recovery key file!";
            return false;
        }

        QByteArray keyData = keyFile.readAll();
        keyFile.close();
        keyData = keyData.trimmed();
        QString recoveryKey = QString::fromUtf8(keyData).trimmed();

        if (recoveryKey.length() != 32) {
            fmWarning() << "Vault: Invalid recovery key format, expected 32 characters, got" << recoveryKey.length();
            return false;
        }

        QRegularExpression keyFormat("^[A-Za-z0-9]{32}$");
        QRegularExpressionMatch match = keyFormat.match(recoveryKey);
        if (!match.hasMatch()) {
            fmWarning() << "Vault: Invalid recovery key format, must contain only letters and numbers";
            return false;
        }

        QString cipher;
        if (!checkPassword(recoveryKey, cipher)) {
            fmWarning() << "Vault: Recovery key verification failed";
            return false;
        }

        password = cipher;
        return true;
    } else {
        QFile localPubKeyfile(keypath);
        if (!localPubKeyfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
            fmCritical() << "Vault: Cannot open local public key file!";
            return false;
        }

        QString strLocalPubKey(localPubKeyfile.readAll());
        localPubKeyfile.close();

        QString strRSACipherFilePath = makeVaultLocalPath(kRSACiphertextFileName);
        QFile rsaCipherfile(strRSACipherFilePath);
        if (!rsaCipherfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
            fmCritical() << "Vault: Cannot open rsa cipher file!";
            return false;
        }

        QString strRsaCipher(rsaCipherfile.readAll());
        rsaCipherfile.close();

        password = rsam::publicKeyDecrypt(strRsaCipher, strLocalPubKey);

        QString temp = "";
        if (!checkPassword(password, temp)) {
            fmCritical() << "Vault: User key error!";
            return false;
        }

        password = temp;
        return true;
    }
}

OperatorCenter *OperatorCenter::getInstance()
{
    static OperatorCenter instance;
    return &instance;
}

OperatorCenter::~OperatorCenter()
{
    fmDebug() << "Vault: OperatorCenter destroyed";
}

Result OperatorCenter::createDirAndFile()
{
    // 创建配置文件目录
    QString strConfigDir = makeVaultLocalPath();
    QDir configDir(strConfigDir);
    if (!configDir.exists()) {
        bool ok = configDir.mkpath(strConfigDir);
        if (!ok) {
            fmCritical() << "Vault: create config dir failed!";
            return { false, tr("Failed to create config dir: %1").arg(strerror(errno)) };
        }
        fmDebug() << "Vault: Config directory created:" << strConfigDir;
    } else {
        fmDebug() << "Vault: Config directory already exists:" << strConfigDir;
    }

    // 创建配置文件,并设置文件权限
    QString strConfigFilePath = strConfigDir + QDir::separator() + kVaultConfigFileName;
    QFile configFile(strConfigFilePath);
    if (!configFile.exists()) {
        // 如果文件不存在，则创建文件，并设置权限
        if (configFile.open(QFileDevice::WriteOnly | QFileDevice::Text)) {
            configFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
            configFile.close();
            fmDebug() << "Vault: Config file created:" << strConfigFilePath;
        } else {
            fmCritical() << "Vault: create config file failed!";
        }
    } else {
        fmDebug() << "Vault: Config file already exists:" << strConfigFilePath;
    }

    // 仅当不是新版本保险箱时才创建RSA文件（向后兼容旧版本保险箱）
    // 新版本保险箱使用LUKS容器，不需要RSA文件
    if (!isNewVaultVersion()) {
        // 创建存放rsa公钥的文件,并设置文件权限
        QString strPriKeyFile = makeVaultLocalPath(kRSAPUBKeyFileName);
        QFile prikeyFile(strPriKeyFile);
        if (!prikeyFile.exists()) {
            if (!prikeyFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
                fmCritical() << "Vault: create rsa private key file failed!";
                return { false, tr("Failed to create rsa private key file: %1").arg(strerror(errno)) };
            }
            prikeyFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
            prikeyFile.close();
            fmDebug() << "Vault: RSA public key file created:" << strPriKeyFile;
        } else {
            fmDebug() << "Vault: RSA public key file already exists:" << strPriKeyFile;
        }

        // 创建存放rsa公钥加密后密文的文件,并设置文件权限
        QString strRsaCiphertext = makeVaultLocalPath(kRSACiphertextFileName);
        QFile rsaCiphertextFile(strRsaCiphertext);
        if (!rsaCiphertextFile.exists()) {
            if (!rsaCiphertextFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
                fmCritical() << "Vault: create rsa ciphertext file failed!";
                return { false, tr("Failed to create rsa ciphertext file: %1").arg(strerror(errno)) };
            }
            rsaCiphertextFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
            rsaCiphertextFile.close();
            fmDebug() << "Vault: RSA ciphertext file created:" << strRsaCiphertext;
        } else {
            fmDebug() << "Vault: RSA ciphertext file already exists:" << strRsaCiphertext;
        }
    } else {
        fmDebug() << "Vault: New version vault detected, skipping RSA file creation";
    }

    // 创建密码提示信息文件,并设置文件权限
    QString strPasswordHintFilePath = makeVaultLocalPath(kPasswordHintFileName);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        fmCritical() << "Vault: create password hint file failed!";
        return { false, tr("Failed to create hint file: %1").arg(strerror(errno)) };
    }
    passwordHintFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    passwordHintFile.close();
    fmDebug() << "Vault: Password hint file created:" << strPasswordHintFilePath;

    return { true };
}

Result OperatorCenter::savePasswordAndPasswordHint(const QString &password, const QString &passwordHint)
{
    fmDebug() << "Vault: Saving password and password hint";

    // encrypt password，write salt and cihper to file
    // random salt
    const QString &strRandomSalt = pbkdf2::createRandomSalt(kRandomSaltLength);
    fmDebug() << "Vault: Random salt created, length:" << strRandomSalt.length();

    // cipher
    const QString &strCiphertext = pbkdf2::pbkdf2EncrypyPassword(password, strRandomSalt, kIteration, kPasswordCipherLength);
    fmDebug() << "Vault: Password encrypted with PBKDF2";

    // salt and cipher
    const QString &strSaltAndCiphertext = strRandomSalt + strCiphertext;
    // save the second encrypt cipher, and update version
    secondSaveSaltAndCiphertext(strSaltAndCiphertext, strRandomSalt, kConfigVaultVersion1050);

    // 保存密码提示信息
    const QString &strPasswordHintFilePath = makeVaultLocalPath(kPasswordHintFileName);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open password hint file failed!";
        return { false, tr("Failed to save hint info: %1").arg(strerror(errno)) };
    }
    QTextStream out2(&passwordHintFile);
    out2 << passwordHint;
    passwordHintFile.close();
    fmDebug() << "Vault: Password hint saved to:" << strPasswordHintFilePath;

    VaultConfig config;
    const QString &useUserPassword = config.get(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant(kConfigKeyNotExist)).toString();
    if (useUserPassword != kConfigKeyNotExist) {
        strCryfsPassword = password;
        fmDebug() << "Vault: Using user password for cryfs";
    } else {
        strCryfsPassword = strSaltAndCiphertext;
        fmDebug() << "Vault: Using encrypted password for cryfs";
    }

    return { true };
}

bool OperatorCenter::createKey(const QString &password, int bytes)
{
    fmDebug() << "Vault: Creating key with bytes:" << bytes;

    // 清空上次的用户密钥
    strUserKey.clear();

    // 创建密钥对
    QString strPriKey("");
    QString strPubKey("");
    rsam::createPublicAndPrivateKey(strPubKey, strPriKey);
    fmDebug() << "Vault: RSA key pair created";

    // 私钥加密
    QString strCipher = rsam::privateKeyEncrypt(password, strPriKey);
    fmDebug() << "Vault: Password encrypted with private key";

    // 将公钥分成两部分（一部分用于保存到本地，一部分生成二维码，提供给用户）
    QString strSaveToLocal("");
    if (strPubKey.length() < 2 * kUserKeyInterceptIndex + bytes) {
        fmCritical() << "Vault: USER_KEY_LENGTH is too long!";
        return false;
    }
    QString strPart1 = strPubKey.mid(0, kUserKeyInterceptIndex);
    QString strPart2 = strPubKey.mid(kUserKeyInterceptIndex, kUserKeyLength);
    QString strPart3 = strPubKey.mid(kUserKeyInterceptIndex + kUserKeyLength);
    strUserKey = strPart2;
    strSaveToLocal = strPart1 + strPart3;
    fmDebug() << "Vault: Public key split into parts, user key length:" << strUserKey.length();

    // 保存部分公钥
    QString publicFilePath = makeVaultLocalPath(kRSAPUBKeyFileName);
    QFile publicFile(publicFilePath);
    if (!publicFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open public key file failure!";
        return false;
    }
    QTextStream out(&publicFile);
    out << strSaveToLocal;
    publicFile.close();
    fmDebug() << "Vault: Partial public key saved to:" << publicFilePath;

    // 保存密文
    QString strCipherFilePath = makeVaultLocalPath(kRSACiphertextFileName);
    QFile cipherFile(strCipherFilePath);
    if (!cipherFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open rsa cipher file failure!";
        return false;
    }
    QTextStream out2(&cipherFile);
    out2 << strCipher;
    cipherFile.close();
    fmDebug() << "Vault: RSA ciphertext saved to:" << strCipherFilePath;

    return true;
}

bool OperatorCenter::checkPassword(const QString &password, QString &cipher)
{
    fmDebug() << "Vault: Checking password";

    // 获得版本信息
    bool isNewVersion = isNewVaultVersion();

    if (isNewVersion) {
        QString containerPath = MasterKeyManager::getContainerPath();

        char masterKeyBuf[64];
        size_t masterKeySize = 64;
        int ret = PasswordManager::exportMasterKey(containerPath.toUtf8().constData(),
                                                   password.toUtf8().constData(),
                                                   masterKeyBuf, &masterKeySize);
        if (ret != 0) {
            return false;
        }

        QByteArray masterKey = QByteArray(masterKeyBuf, 64);
        VaultConfig config;
        QString creationType = config.getVaultCreationType();

        QByteArray cryfsPassword;
        if (creationType == kConfigValueVaultCreationTypeNew) {
            cryfsPassword = masterKey;
        } else if (creationType == kConfigValueVaultCreationTypeMigrated) {
            cryfsPassword = masterKey;
            while (cryfsPassword.endsWith('\0')) {
                cryfsPassword.chop(1);
            }
        } else {
            cryfsPassword = masterKey;
        }

        cipher = QString::fromLatin1(cryfsPassword.toBase64());
        return true;
    }

    VaultConfig config;
    const QString &strVersion = config.get(kConfigNodeName, kConfigKeyVersion).toString();

    if ((kConfigVaultVersion == strVersion) || (kConfigVaultVersion1050 == strVersion)) {   // 如果是新版本，验证第二次加密的结果
        // 获得本地盐及密文
        QString strSaltAndCipher = config.get(kConfigNodeName, kConfigKeyCipher).toString();
        QString strSalt = strSaltAndCipher.mid(0, kRandomSaltLength);
        QString strCipher = strSaltAndCipher.mid(kRandomSaltLength);
        // pbkdf2第一次加密密码,获得密文
        QString strNewCipher = pbkdf2::pbkdf2EncrypyPassword(password, strSalt, kIteration, kPasswordCipherLength);
        // 组合密文和盐值
        QString strNewSaltAndCipher = strSalt + strNewCipher;
        // pbkdf2第二次加密密码，获得密文
        QString strNewCipher2 = pbkdf2::pbkdf2EncrypyPassword(strNewSaltAndCipher, strSalt, kIterationTwo, kPasswordCipherLength);

        if (strCipher != strNewCipher2) {
            fmWarning() << "Vault: password error!";
            return false;
        }

        const QString &useUserPassword = config.get(kConfigNodeName, kConfigKeyUseUserPassWord, QVariant(kConfigKeyNotExist)).toString();
        if (useUserPassword != kConfigKeyNotExist) {
            cipher = password;
            fmDebug() << "Vault: Using user password";
        } else {
            cipher = strNewSaltAndCipher;
            fmDebug() << "Vault: Using encrypted password";
        }
    } else {   // 如果是旧版本，验证第一次加密的结果
        fmDebug() << "Vault: Using old version password verification";
        // 获得本地盐及密文
        QString strfilePath = makeVaultLocalPath(kPasswordFileName);
        QFile file(strfilePath);
        if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            fmCritical() << "Vault: open pbkdf2cipher file failed!";
            return false;
        }
        QString strSaltAndCipher = QString(file.readAll());
        file.close();
        QString strSalt = strSaltAndCipher.mid(0, kRandomSaltLength);
        QString strCipher = strSaltAndCipher.mid(kRandomSaltLength);

        // pbkdf2加密密码,获得密文
        QString strNewCipher = pbkdf2::pbkdf2EncrypyPassword(password, strSalt, kIteration, kPasswordCipherLength);
        QString strNewSaltAndCipher = strSalt + strNewCipher;
        if (strNewSaltAndCipher != strSaltAndCipher) {
            fmCritical() << "Vault: password error!";
            return false;
        }

        cipher = strNewSaltAndCipher;

        // 保存第二次加密后的密文,并更新保险箱版本信息
        if (!secondSaveSaltAndCiphertext(strNewSaltAndCipher, strSalt, kConfigVaultVersion)) {
            fmCritical() << "Vault Error: the second encrypt failed!";
            return false;
        }

        // 删除旧版本遗留的密码文件
        QFile::remove(strfilePath);
        fmDebug() << "Vault: Old password file removed:" << strfilePath;
    }
    return true;
}

bool OperatorCenter::checkUserKey(const QString &userKey, QString &cipher)
{
    // 新版本保险箱不支持RSA用户密钥验证，应使用checkPassword()验证恢复密钥
    if (isNewVaultVersion()) {
        fmWarning() << "Vault: checkUserKey called on new version vault, not supported. Use checkPassword() for recovery key verification.";
        return false;
    }

    if (userKey.length() != kUserKeyLength) {
        fmCritical() << "Vault: user key length error!";
        return false;
    }

    // 结合本地公钥和用户密钥，还原完整公钥
    QString strLocalPubKeyFilePath = makeVaultLocalPath(kRSAPUBKeyFileName);
    QFile localPubKeyfile(strLocalPubKeyFilePath);
    if (!localPubKeyfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        fmCritical() << "Vault: cant't open local public key file!";
        return false;
    }
    QString strLocalPubKey(localPubKeyfile.readAll());
    localPubKeyfile.close();
    fmDebug() << "Vault: Local public key loaded, length:" << strLocalPubKey.length();

    QString strNewPubKey = strLocalPubKey.insert(kUserKeyInterceptIndex, userKey);
    fmDebug() << "Vault: Complete public key reconstructed";

    // 利用完整公钥解密密文，得到密码
    QString strRSACipherFilePath = makeVaultLocalPath(kRSACiphertextFileName);
    QFile rsaCipherfile(strRSACipherFilePath);
    if (!rsaCipherfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        fmCritical() << "Vault: cant't open rsa cipher file!";
        return false;
    }
    QString strRsaCipher(rsaCipherfile.readAll());
    rsaCipherfile.close();
    fmDebug() << "Vault: RSA cipher loaded, length:" << strRsaCipher.length();

    QString strNewPassword = rsam::publicKeyDecrypt(strRsaCipher, strNewPubKey);
    fmDebug() << "Vault: Password decrypted from RSA cipher";

    // 判断密码的正确性，如果密码正确，则用户密钥正确，否则用户密钥错误
    if (!checkPassword(strNewPassword, cipher)) {
        fmCritical() << "Vault: user key error!";
        return false;
    }

    return true;
}

QString OperatorCenter::getUserKey()
{
    return strUserKey;
}

bool OperatorCenter::getPasswordHint(QString &passwordHint)
{
    QString strPasswordHintFilePath = makeVaultLocalPath(kPasswordHintFileName);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        fmCritical() << "Vault: open password hint file failed!";
        return false;
    }
    passwordHint = QString(passwordHintFile.readAll());
    passwordHintFile.close();
    fmDebug() << "Vault: Password hint loaded, length:" << passwordHint.length();

    return true;
}

QString OperatorCenter::getSaltAndPasswordCipher()
{
    return strCryfsPassword;
}

void OperatorCenter::clearSaltAndPasswordCipher()
{
    strCryfsPassword.clear();
}

QString OperatorCenter::getEncryptDirPath()
{
    return makeVaultLocalPath(kVaultEncrypyDirName);
}

QString OperatorCenter::getdecryptDirPath()
{
    return makeVaultLocalPath(kVaultDecryptDirName);
}

QStringList OperatorCenter::getConfigFilePath()
{
    QStringList lstPath;

    lstPath << makeVaultLocalPath(kPasswordFileName);
    lstPath << makeVaultLocalPath(kRSAPUBKeyFileName);
    lstPath << makeVaultLocalPath(kRSACiphertextFileName);
    lstPath << makeVaultLocalPath(kPasswordHintFileName);

    return lstPath;
}

QString OperatorCenter::autoGeneratePassword(int length)
{
    if (length < 3) {
        fmWarning() << "Vault: Password length too short:" << length;
        return "";
    }

    ::srand(uint(QTime(0, 0, 0).secsTo(QTime::currentTime())));

    QString strPassword("");

    QString strNum("0123456789");
    strPassword += strNum.at(::rand() % 10);

    QString strSpecialChar("`~!@#$%^&*");
    strPassword += strSpecialChar.at(::rand() % 10);

    QString strABC("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    strPassword += strABC.at(::rand() % 10);

    QString strAllChar = strNum + strSpecialChar + strABC;
    int nCount = length - 3;
    for (int i = 0; i < nCount; ++i) {
        strPassword += strAllChar.at(::rand() % 52);
    }

    fmDebug() << "Vault: Generated password success";
    return strPassword;
}

bool OperatorCenter::getRootPassword()
{
    // 判断当前是否是管理员登陆
    bool res = runCmd("id -un");   // file path is fixed. So write cmd direct
    if (res && standOutput.trimmed() == "root") {
        fmDebug() << "Vault: Already running as root";
        return true;
    }

    if (false == executeProcess("sudo whoami")) {
        fmWarning() << "Vault: Failed to get root privileges";
        return false;
    }

    return true;
}

int OperatorCenter::executionShellCommand(const QString &strCmd, QStringList &lstShellOutput)
{
    FILE *fp;

    std::string sCmd = strCmd.toStdString();
    const char *cmd = sCmd.c_str();

    // 命令为空
    if (strCmd.isEmpty()) {
        fmCritical() << "Vault: the shell cmd is empty!";
        return -1;
    }

    if ((fp = popen(cmd, "r")) == nullptr) {
        perror("popen");
        fmCritical() << QString("Vault Error: popen error: %s").arg(strerror(errno));
        return -1;
    } else {
        char buf[kBuffterMaxLine] = { '\0' };
        while (fgets(buf, sizeof(buf), fp)) {   // 获得每行输出
            QString strLineOutput(buf);
            if (strLineOutput.endsWith('\n'))
                strLineOutput.chop(1);
            lstShellOutput.push_back(strLineOutput);
        }

        int res;
        if ((res = pclose(fp)) == -1) {
            fmCritical() << "Vault: close popen file pointer fp failed!";
            return res;
        } else if (res == 0) {
            return res;
        } else {
            fmCritical() << QString("Vault: popen res is : %1").arg(res);
            return res;
        }
    }
}

Result OperatorCenter::savePasswordToKeyring(const QString &password)
{
    GError *error = Q_NULLPTR;
    SecretService *service = Q_NULLPTR;
    QByteArray baPassword = password.toLatin1();
    const char *cPassword = baPassword.data();
    // Create a password struceture
    SecretValue *value = secret_value_new_full(g_strdup(cPassword), strlen(cPassword), "text/plain", (GDestroyNotify)secret_password_free);
    // Obtain password service synchronously
    service = secret_service_get_sync(SECRET_SERVICE_NONE, Q_NULLPTR, &error);
    if (error == Q_NULLPTR) {
        GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        // Get the currently logged in user information
        char *userName = getlogin();
        g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
        g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));
        secret_service_store_sync(service, Q_NULLPTR, attributes, Q_NULLPTR, "uos cryfs password", value, Q_NULLPTR, &error);
        g_hash_table_destroy(attributes);
    }
    secret_value_unref(value);

    if (error != Q_NULLPTR) {
        fmCritical() << "Vault: Store password failed! error :" << QString(error->message);
        return { false, tr("Save password failed: %1").arg(error->message) };
    }

    fmInfo() << "Vault: Store password end!";

    return { true };
}

QString OperatorCenter::passwordFromKeyring()
{
    QString result { "" };

    GError *error = Q_NULLPTR;
    SecretService *service = Q_NULLPTR;
    char *userName = getlogin();
    GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
    g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));

    service = secret_service_get_sync(SECRET_SERVICE_NONE, Q_NULLPTR, &error);

    SecretValue *value_read = secret_service_lookup_sync(service, Q_NULLPTR, attributes, Q_NULLPTR, &error);
    gsize length;
    const gchar *passwd = secret_value_get(value_read, &length);
    if (length > 0) {
        result = QString(passwd);
    }

    secret_value_unref(value_read);
    g_hash_table_destroy(attributes);

    return result;
}

void OperatorCenter::removeVault(const QString &basePath)
{
    fmDebug() << "Vault: Removing vault from base path";

    if (basePath.isEmpty()) {
        fmWarning() << "Vault: Base path is empty, cannot remove vault";
        return;
    }

    QtConcurrent::run([this, basePath]() {
        fmDebug() << "Vault: Starting vault removal in background thread";
        int filesCount { 0 };
        int removedFileCount { 0 };
        int removedDirCount { 0 };
        if (statisticsFilesInDir(basePath, &filesCount)) {
            filesCount++;   // the basePath dir
            removeDir(basePath, filesCount, &removedFileCount, &removedDirCount);
            fmDebug() << "Vault: Vault removal completed";
        }
    });
}

bool OperatorCenter::isNewVaultVersion() const
{
    // 判断是否是透明加密
    VaultConfig config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        return false;
    }
    // 构建LUKS容器文件路径
    QString containerPath = kVaultBasePath + QString("/password_container.bin");

    // 检查文件是否存在
    return QFile::exists(containerPath);
}

/*!
 * \brief 通过旧密码将老版本保险箱迁移到新密码管理方案
 *
 * 流程：
 * 1. 使用老版本 pbkdf2 流程验证旧密码；
 * 2. 使用旧密码通过 MasterKeyManager 生成主密钥（补零方案）；
 * 3. 创建 LUKS 容器并使用新密码添加用户密码 KeySlot；
 * 4. 为新方案准备恢复密钥（复用现有逻辑：保持恢复密钥不变或生成新的恢复密钥）；
 * 5. 在配置中标记为 \"migrated\"，后续解锁走新方案。
 */
bool OperatorCenter::migrateOldVaultByPassword(const QString &oldPassword,
                                               const QString &newPassword,
                                               QString &outRecoveryKey)
{
    if (isNewVaultVersion()) {
        fmWarning() << "Vault: migrateOldVaultByPassword called on new version vault, skip";
        return false;
    }

    QString cipher;
    if (!checkPassword(oldPassword, cipher)) {
        fmWarning() << "Vault: migrateOldVaultByPassword old password verification failed";
        return false;
    }

    QByteArray oldPasswordBytes = oldPassword.toUtf8();
    QByteArray masterKey = MasterKeyManager::generateMasterKeyFromPassword(oldPasswordBytes);
    if (masterKey.isEmpty()) {
        fmWarning() << "Vault: migrateOldVaultByPassword failed to generate master key from old password";
        return false;
    }

    QString containerPath = MasterKeyManager::getContainerPath();
    // 先创建密码容器文件，确保 crypt_init 可以正常打开
    int fileRet = PasswordManager::createPasswordContainerFile(containerPath.toUtf8().constData());
    if (fileRet != 0) {
        fmWarning() << "Vault: migrateOldVaultByPassword failed to create password container file";
        return false;
    }
    int slotID = 0;
    int ret = PasswordManager::createLuksContainer(containerPath.toUtf8().constData(),
                                                   masterKey.constData(),
                                                   static_cast<size_t>(masterKey.size()),
                                                   newPassword.toUtf8().constData(),
                                                   slotID);
    if (ret != 0) {
        fmWarning() << "Vault: migrateOldVaultByPassword failed to create LUKS container, ret =" << ret;
        return false;
    }

    QString recoveryKey = generateRecoveryKeyForNewVault();
    if (recoveryKey.isEmpty()) {
        fmWarning() << "Vault: migrateOldVaultByPassword failed to generate recovery key";
        return false;
    }

    int recoveryKeySlotId = 0;
    ret = PasswordManager::addNewPassword(containerPath.toUtf8().constData(),
                                          newPassword.toUtf8().constData(),
                                          recoveryKey.toUtf8().constData(),
                                          recoveryKeySlotId);
    if (ret != 0) {
        fmWarning() << "Vault: migrateOldVaultByPassword failed to add recovery key to LUKS";
        return false;
    }

    VaultConfig config;
    config.setVaultCreationType(kConfigValueVaultCreationTypeMigrated);

    outRecoveryKey = recoveryKey;
    return true;
}

/*!
 * \brief 通过恢复密钥将老版本保险箱迁移到新密码管理方案
 *
 * 恢复密钥场景下，调用方需要先从密钥文件中解析出恢复密钥字符串。
 * 这里直接使用恢复密钥作为“等效旧密码”，后续流程与 migrateOldVaultByPassword 类似。
 */
bool OperatorCenter::migrateOldVaultByRecoveryKey(const QString &recoveryKey,
                                                  const QString &newPassword,
                                                  QString &outRecoveryKey)
{
    // 仅在老版本保险箱上执行迁移
    if (isNewVaultVersion()) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey called on new version vault, skip";
        return false;
    }

    if (recoveryKey.isEmpty()) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey recovery key is empty";
        return false;
    }

    QString cipher;
    if (!checkPassword(recoveryKey, cipher)) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey recovery key verification failed";
        return false;
    }

    QByteArray recoveryKeyBytes = recoveryKey.toUtf8();
    QByteArray masterKey = MasterKeyManager::generateMasterKeyFromPassword(recoveryKeyBytes);
    if (masterKey.isEmpty()) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey failed to generate master key from recovery key";
        return false;
    }

    // 3. 创建 LUKS 容器并添加新密码
    QString containerPath = MasterKeyManager::getContainerPath();
    // 先创建密码容器文件，确保 crypt_init 可以正常打开
    int fileRet = PasswordManager::createPasswordContainerFile(containerPath.toUtf8().constData());
    if (fileRet != 0) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey failed to create password container file";
        return false;
    }

    int slotID = 0;
    int ret = PasswordManager::createLuksContainer(containerPath.toUtf8().constData(),
                                                   masterKey.constData(),
                                                   static_cast<size_t>(masterKey.size()),
                                                   newPassword.toUtf8().constData(),
                                                   slotID);
    if (ret != 0) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey failed to create LUKS container, ret =" << ret;
        return false;
    }

    QString newRecoveryKey = generateRecoveryKeyForNewVault();
    if (newRecoveryKey.isEmpty()) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey failed to generate new recovery key";
        return false;
    }

    int recoveryKeySlotId = 0;
    ret = PasswordManager::addNewPassword(containerPath.toUtf8().constData(),
                                          newPassword.toUtf8().constData(),
                                          newRecoveryKey.toUtf8().constData(),
                                          recoveryKeySlotId);
    if (ret != 0) {
        fmWarning() << "Vault: migrateOldVaultByRecoveryKey failed to add new recovery key to LUKS";
        return false;
    }

    VaultConfig config;
    config.setVaultCreationType(kConfigValueVaultCreationTypeMigrated);

    outRecoveryKey = newRecoveryKey;
    return true;
}

/*!
 * \brief 通过验证密码将老版本保险箱迁移到新密码管理方案（不重置密码）
 *
 * 这是从老密码方案（pbkdf2）迁移到新密码管理方案（LUKS）的特定流程。
 * 与 migrateOldVaultByPassword 的区别是：使用老密码作为 LUKS 容器的用户密码，用户不需要重置密码。
 *
 * 流程：
 * 1. 使用老版本 pbkdf2 流程验证老密码；
 * 2. 使用老密码通过 MasterKeyManager 生成主密钥（补零方案）；
 * 3. 创建 LUKS 容器并使用老密码添加用户密码 KeySlot；
 * 4. 生成新的恢复密钥并添加到 KeySlot 1；
 * 5. 在配置中标记为 "migrated"，后续解锁走新方案。
 */
bool OperatorCenter::upgradeOldVaultByPassword(const QString &oldPassword, QString &outRecoveryKey)
{
    // 仅在老版本保险箱上执行迁移
    if (isNewVaultVersion()) {
        fmWarning() << "Vault: upgradeOldVaultByPassword called on new version vault, skip";
        return false;
    }

    // 1. 使用旧版本流程验证老密码（checkPassword 内部会区分新旧版本）
    QString cipher;
    if (!checkPassword(oldPassword, cipher)) {
        fmWarning() << "Vault: upgradeOldVaultByPassword old password verification failed";
        return false;
    }

    // 2. 使用老密码生成主密钥（补零方案）
    QByteArray oldPasswordBytes = oldPassword.toUtf8();
    QByteArray masterKey = MasterKeyManager::generateMasterKeyFromPassword(oldPasswordBytes);
    if (masterKey.isEmpty()) {
        fmWarning() << "Vault: upgradeOldVaultByPassword failed to generate master key from old password";
        return false;
    }

    // 3. 创建 LUKS 容器并使用老密码添加用户密码 KeySlot（注意：使用 oldPassword 而不是 newPassword）
    QString containerPath = MasterKeyManager::getContainerPath();
    // 先创建密码容器文件，确保 crypt_init 可以正常打开
    int fileRet = PasswordManager::createPasswordContainerFile(containerPath.toUtf8().constData());
    if (fileRet != 0) {
        fmWarning() << "Vault: upgradeOldVaultByPassword failed to create password container file";
        return false;
    }
    int slotID = 0;
    int ret = PasswordManager::createLuksContainer(containerPath.toUtf8().constData(),
                                                   masterKey.constData(),
                                                   static_cast<size_t>(masterKey.size()),
                                                   oldPassword.toUtf8().constData(),  // 使用 oldPassword
                                                   slotID);
    if (ret != 0) {
        fmWarning() << "Vault: upgradeOldVaultByPassword failed to create LUKS container, ret =" << ret;
        return false;
    }

    // 4. 准备恢复密钥：如果已经预生成（用户在保存密钥文件页面时生成的），则使用它；否则生成新的
    QString recoveryKey = getRecoveryKey();
    if (recoveryKey.isEmpty() || recoveryKey.length() != 32) {
        recoveryKey = generateRecoveryKeyForNewVault();
        if (recoveryKey.isEmpty()) {
            fmWarning() << "Vault: upgradeOldVaultByPassword failed to generate recovery key";
            return false;
        }
    }

    // 将恢复密钥添加到 LUKS KeySlot 1
    int recoveryKeySlotId = 0;
    ret = PasswordManager::addNewPassword(containerPath.toUtf8().constData(),
                                          oldPassword.toUtf8().constData(),  // 使用 oldPassword 作为 existingPassword
                                          recoveryKey.toUtf8().constData(),
                                          recoveryKeySlotId);
    if (ret != 0) {
        fmWarning() << "Vault: upgradeOldVaultByPassword failed to add recovery key to LUKS";
        return false;
    }

    // 5. 更新配置，标记为迁移后的新方案
    VaultConfig config;
    config.setVaultCreationType(kConfigValueVaultCreationTypeMigrated);

    outRecoveryKey = recoveryKey;
    return true;
}


bool OperatorCenter::resetPasswordByOldPassword(const QString &oldPassword, const QString &newPassword, const QString &passwordHint)
{
    // 重置密码只支持新版本保险箱，老版本保险箱需要通过升级流程迁移
    if (!isNewVaultVersion()) {
        fmWarning() << "Vault: Cannot reset password for old version vault. Please upgrade the vault first.";
        return false;
    }

    QString containerPath = MasterKeyManager::getContainerPath();

    int newKeySlotId = 0;
    int ret = PasswordManager::changePassword(containerPath.toUtf8().constData(),
                                              oldPassword.toUtf8().constData(),
                                              newPassword.toUtf8().constData(),
                                              newKeySlotId);
    if (ret != 0) {
        fmWarning() << "Vault: Old password verification failed or failed to change password";
        return false;
    }

    // 更新密码提示信息（可选，允许为空，行为与创建保险箱时一致）
    const QString &strPasswordHintFilePath = makeVaultLocalPath(kPasswordHintFileName);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open password hint file failed when resetting password!";
        return false;
    }
    QTextStream out2(&passwordHintFile);
    out2 << passwordHint;
    passwordHintFile.close();

    // 恢复密钥保持不变，不需要修改
    return true;
}

bool OperatorCenter::resetPasswordByRecoveryKey(const QString &recoveryKey, const QString &newPassword, const QString &passwordHint)
{
    // 重置密码只支持新版本保险箱，老版本保险箱需要通过升级流程迁移
    if (!isNewVaultVersion()) {
        fmWarning() << "Vault: Cannot reset password for old version vault. Please upgrade the vault first.";
        return false;
    }

    QString containerPath = MasterKeyManager::getContainerPath();

    // 1. 验证恢复密钥格式（确保是32个字符）
    if (recoveryKey.length() != 32) {
        fmWarning() << "Vault: Recovery key length is invalid, expected 32, got" << recoveryKey.length();
        return false;
    }

    // 2. 验证恢复密钥（恢复密钥永远在槽1，直接尝试槽1）
    QByteArray recoveryKeyBytes = recoveryKey.toLatin1();
    char masterKeyBuf[64];
    size_t masterKeySize = 64;
    int ret = PasswordManager::exportMasterKeyByKeyslot(containerPath.toUtf8().constData(),
                                                        recoveryKeyBytes.constData(),
                                                        1,  // 恢复密钥在槽1
                                                        masterKeyBuf, &masterKeySize);
    if (ret != 0) {
        fmWarning() << "Vault: Recovery key verification failed";
        return false;
    }

    // 3. 恢复密钥永远在槽1
    int recoveryKeyKeyslot = 1;

    // 4. 使用恢复密钥添加新密码到新的KeySlot
    int newKeySlotId = 0;
    ret = PasswordManager::addNewPasswordByKeyslot(
        containerPath.toUtf8().constData(),
        recoveryKeyBytes.constData(),
        1,  // 恢复密钥在槽1
        newPassword.toUtf8().constData(),
        newKeySlotId);

    if (ret != 0) {
        fmWarning() << "Vault: Failed to add new password";
        return false;
    }

    // 5. 删除旧密码的KeySlot
    int commonKeyslots[] = { 0, 1, 2 };
    for (int i = 0; i < 3; i++) {
        int keyslot = commonKeyslots[i];
        if (keyslot != newKeySlotId && keyslot != recoveryKeyKeyslot) {
            PasswordManager::deleteKeyslot(containerPath.toUtf8().constData(), keyslot);
        }
    }

    // 更新密码提示信息（可选，允许为空，行为与创建保险箱时一致）
    const QString &strPasswordHintFilePath = makeVaultLocalPath(kPasswordHintFileName);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault: open password hint file failed when resetting password!";
        return false;
    }
    QTextStream out2(&passwordHintFile);
    out2 << passwordHint;
    passwordHintFile.close();

    // 恢复密钥保持不变，不需要修改
    return true;
}
