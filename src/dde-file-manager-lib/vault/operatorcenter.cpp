/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "operatorcenter.h"
#include "openssl-operator/pbkdf2.h"
#include "openssl-operator/rsam.h"
#include "qrencode/qrencode.h"
#include "vaultconfig.h"

#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTime>
#include <QtGlobal>
#include <QProcess>

OperatorCenter::OperatorCenter(QObject *parent)
    : QObject(parent), m_strCryfsPassword(""), m_strUserKey(""), m_standOutput("")
{
}

QString OperatorCenter::makeVaultLocalPath(const QString &before, const QString &behind)
{
    return VAULT_BASE_PATH
            //            + QDir::separator() + CONFIG_DIR_NAME
            + (before.isEmpty() ? QString("") : QDir::separator()) + before
            + (behind.isEmpty() ? QString("") : QDir::separator()) + behind;
}

bool OperatorCenter::runCmd(const QString &cmd)
{
    QProcess process;
    int mescs = 10000;
    if (cmd.startsWith(ROOT_PROXY)) {
        mescs = -1;
    }
    process.start(cmd);

    bool res = process.waitForFinished(mescs);
    m_standOutput = process.readAllStandardOutput();
    int exitCode = process.exitCode();
    if (cmd.startsWith(ROOT_PROXY) && (exitCode == 127 || exitCode == 126)) {
        QString strOut = "Run \'" + cmd + "\' fauled: Password Error! " + QString::number(exitCode) + "\n";
        qDebug() << strOut;
        return false;
    }

    if (res == false) {
        QString strOut = "Run \'" + cmd + "\' failed\n";
        qDebug() << strOut;
    }

    return res;
}

bool OperatorCenter::executeProcess(const QString &cmd)
{
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd);
    }

    runCmd("id -un");
    if (m_standOutput.trimmed() == "root") {
        return runCmd(cmd);
    }

    QString newCmd = QString(ROOT_PROXY) + " \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd);
}

bool OperatorCenter::secondSaveSaltAndCiphertext(const QString &ciphertext, const QString &salt, const char *vaultVersion)
{
    // 密文
    QString strCiphertext = pbkdf2::pbkdf2EncrypyPassword(ciphertext, salt, ITERATION_TWO, PASSWORD_CIPHER_LENGTH);
    if (strCiphertext.isEmpty())
        return false;
    // 写入文件
    QString strSaltAndCiphertext = salt + strCiphertext;
    VaultConfig config;
    config.set(CONFIG_NODE_NAME, CONFIG_KEY_CIPHER, QVariant(strSaltAndCiphertext));
    // 更新保险箱版本信息
    config.set(CONFIG_NODE_NAME, CONFIG_KEY_VERSION, QVariant(vaultVersion));

    return true;
}

bool OperatorCenter::createKeyNew(const QString &password)
{
    m_strPubKey.clear();
    QString strPriKey("");
    rsam::createPublicAndPrivateKey(m_strPubKey, strPriKey);

    // 私钥加密
    QString strCipher = rsam::privateKeyEncrypt(password, strPriKey);

    // 验证公钥长度
    if (m_strPubKey.length() < 2 * USER_KEY_INTERCEPT_INDEX + 32) {
        qDebug() << "USER_KEY_LENGTH is to long!";
        m_strPubKey.clear();
        return false;
    }

    // 保存密文
    QString strCipherFilePath = makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    QFile cipherFile(strCipherFilePath);
    if (!cipherFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "open rsa cipher file failure!";
        return false;
    }
    QTextStream out2(&cipherFile);
    out2 << strCipher;
    cipherFile.close();

    return true;
}

bool OperatorCenter::saveKey(QString key, QString path)
{
    // 保存部分公钥
    QString publicFilePath = path;
    QFile publicFile(publicFilePath);
    if (!publicFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "open public key file failure!";
        return false;
    }
    publicFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    QTextStream out(&publicFile);
    out << key;
    publicFile.close();
    return true;
}

QString OperatorCenter::getPubKey()
{
    return m_strPubKey;
}

bool OperatorCenter::verificationRetrievePassword(const QString keypath, QString &password)
{
    QFile localPubKeyfile(keypath);
    if (!localPubKeyfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug() << "cant't open local public key file!";
        return false;
    }

    QString strLocalPubKey(localPubKeyfile.readAll());
    localPubKeyfile.close();

    // 利用完整公钥解密密文，得到密码
    QString strRSACipherFilePath = makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    QFile rsaCipherfile(strRSACipherFilePath);
    if (!rsaCipherfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug() << "cant't open rsa cipher file!";
        return false;
    }

    QString strRsaCipher(rsaCipherfile.readAll());
    rsaCipherfile.close();

    password = rsam::publicKeyDecrypt(strRsaCipher, strLocalPubKey);

    // 判断密码的正确性，如果密码正确，则用户密钥正确，否则用户密钥错误
    QString temp = "";
    if (!checkPassword(password, temp)) {
        qDebug() << "user key error!";
        return false;
    }

    return true;
}

OperatorCenter *OperatorCenter::getInstance()
{
    static OperatorCenter instance;
    return &instance;
}

OperatorCenter::~OperatorCenter()
{
}

bool OperatorCenter::createDirAndFile()
{
    // 创建配置文件目录
    QString strConfigDir = makeVaultLocalPath();
    QDir configDir(strConfigDir);
    if (!configDir.exists()) {
        bool ok = configDir.mkpath(strConfigDir);
        if (!ok) {
            qDebug() << "create config dir failure!";
            return false;
        }
    }

    // 创建配置文件,并设置文件权限
    QString strConfigFilePath = strConfigDir + QDir::separator() + VAULT_CONFIG_FILE_NAME;
    QFile configFile(strConfigFilePath);
    if (!configFile.exists()) {
        // 如果文件不存在，则创建文件，并设置权限
        if (configFile.open(QFileDevice::WriteOnly | QFileDevice::Text)) {
            configFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
            configFile.close();
        } else {
            qInfo() << "保险箱：创建配置文件失败！";
        }
    }

    // 创建存放rsa公钥的文件,并设置文件权限
    QString strPriKeyFile = makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    QFile prikeyFile(strPriKeyFile);
    if (!prikeyFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "create rsa private key file failure!";
        return false;
    }
    prikeyFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    prikeyFile.close();

    // 创建存放rsa公钥加密后密文的文件,并设置文件权限
    QString strRsaCiphertext = makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    QFile rsaCiphertextFile(strRsaCiphertext);
    if (!rsaCiphertextFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "create rsa ciphertext file failure!";
        return false;
    }
    rsaCiphertextFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    rsaCiphertextFile.close();

    // 创建密码提示信息文件,并设置文件权限
    QString strPasswordHintFilePath = makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "create password hint file failure!";
        return false;
    }
    passwordHintFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    passwordHintFile.close();

    return true;
}

bool OperatorCenter::saveSaltAndCiphertext(const QString &password, const QString &passwordHint)
{
    // 加密密码，并将盐和密文写入密码文件
    // 随机盐
    QString strRandomSalt = pbkdf2::createRandomSalt(RANDOM_SALT_LENGTH);
    // 密文
    QString strCiphertext = pbkdf2::pbkdf2EncrypyPassword(password, strRandomSalt, ITERATION, PASSWORD_CIPHER_LENGTH);
    // 组合盐值与密文
    QString strSaltAndCiphertext = strRandomSalt + strCiphertext;

    // 保存第二次加密后的密文,并更新保险箱版本信息
    secondSaveSaltAndCiphertext(strSaltAndCiphertext, strRandomSalt, CONFIG_VAULT_VERSION_1050);

    // 保存密码提示信息
    QString strPasswordHintFilePath = makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "write password hint failure";
        return false;
    }
    QTextStream out2(&passwordHintFile);
    out2 << passwordHint;
    passwordHintFile.close();

    // 缓存cryfs密码
    m_strCryfsPassword = strSaltAndCiphertext;

    return true;
}

bool OperatorCenter::createKey(const QString &password, int bytes)
{
    // 清空上次的用户密钥
    m_strUserKey.clear();

    // 创建密钥对
    QString strPriKey("");
    QString strPubKey("");
    rsam::createPublicAndPrivateKey(strPubKey, strPriKey);

    // 私钥加密
    QString strCipher = rsam::privateKeyEncrypt(password, strPriKey);

    // 将公钥分成两部分（一部分用于保存到本地，一部分生成二维码，提供给用户）
    QString strSaveToLocal("");
    if (strPubKey.length() < 2 * USER_KEY_INTERCEPT_INDEX + bytes) {
        qDebug() << "USER_KEY_LENGTH is to long!";
        return false;
    }
    QString strPart1 = strPubKey.mid(0, USER_KEY_INTERCEPT_INDEX);
    QString strPart2 = strPubKey.mid(USER_KEY_INTERCEPT_INDEX, USER_KEY_LENGTH);
    QString strPart3 = strPubKey.mid(USER_KEY_INTERCEPT_INDEX + USER_KEY_LENGTH);
    m_strUserKey = strPart2;
    strSaveToLocal = strPart1 + strPart3;

    // 保存部分公钥
    QString publicFilePath = makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    QFile publicFile(publicFilePath);
    if (!publicFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "open public key file failure!";
        return false;
    }
    QTextStream out(&publicFile);
    out << strSaveToLocal;
    publicFile.close();

    // 保存密文
    QString strCipherFilePath = makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    QFile cipherFile(strCipherFilePath);
    if (!cipherFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "open rsa cipher file failure!";
        return false;
    }
    QTextStream out2(&cipherFile);
    out2 << strCipher;
    cipherFile.close();

    return true;
}

bool OperatorCenter::checkPassword(const QString &password, QString &cipher)
{
    // 获得版本信息
    VaultConfig config;
    QString strVersion = config.get(CONFIG_NODE_NAME, CONFIG_KEY_VERSION).toString();

    if ((CONFIG_VAULT_VERSION == strVersion) || (CONFIG_VAULT_VERSION_1050 == strVersion)) {   // 如果是新版本，验证第二次加密的结果
        // 获得本地盐及密文
        QString strSaltAndCipher = config.get(CONFIG_NODE_NAME, CONFIG_KEY_CIPHER).toString();
        QString strSalt = strSaltAndCipher.mid(0, RANDOM_SALT_LENGTH);
        QString strCipher = strSaltAndCipher.mid(RANDOM_SALT_LENGTH);
        // pbkdf2第一次加密密码,获得密文
        QString strNewCipher = pbkdf2::pbkdf2EncrypyPassword(password, strSalt, ITERATION, PASSWORD_CIPHER_LENGTH);
        // 组合密文和盐值
        QString strNewSaltAndCipher = strSalt + strNewCipher;
        // pbkdf2第二次加密密码，获得密文
        QString strNewCipher2 = pbkdf2::pbkdf2EncrypyPassword(strNewSaltAndCipher, strSalt, ITERATION_TWO, PASSWORD_CIPHER_LENGTH);

        if (strCipher != strNewCipher2) {
            qDebug() << "password error!";
            return false;
        }
        cipher = strNewSaltAndCipher;
    } else {   // 如果是旧版本，验证第一次加密的结果
        // 获得本地盐及密文
        QString strfilePath = makeVaultLocalPath(PASSWORD_FILE_NAME);
        QFile file(strfilePath);
        if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qDebug() << "open pbkdf2cipher file failure!";
            return false;
        }
        QString strSaltAndCipher = QString(file.readAll());
        file.close();
        QString strSalt = strSaltAndCipher.mid(0, RANDOM_SALT_LENGTH);
        QString strCipher = strSaltAndCipher.mid(RANDOM_SALT_LENGTH);

        // pbkdf2加密密码,获得密文
        QString strNewCipher = pbkdf2::pbkdf2EncrypyPassword(password, strSalt, ITERATION, PASSWORD_CIPHER_LENGTH);
        QString strNewSaltAndCipher = strSalt + strNewCipher;
        if (strNewSaltAndCipher != strSaltAndCipher) {
            qDebug() << "password error!";
            return false;
        }

        cipher = strNewSaltAndCipher;

        // 保存第二次加密后的密文,并更新保险箱版本信息
        if (!secondSaveSaltAndCiphertext(strNewSaltAndCipher, strSalt, CONFIG_VAULT_VERSION)) {
            qDebug() << "第二次加密密文失败！";
            return false;
        }

        // 删除旧版本遗留的密码文件
        QFile::remove(strfilePath);
    }
    return true;
}

bool OperatorCenter::checkUserKey(const QString &userKey, QString &cipher)
{
    if (userKey.length() != USER_KEY_LENGTH) {
        qDebug() << "user key length error!";
        return false;
    }

    // 结合本地公钥和用户密钥，还原完整公钥
    QString strLocalPubKeyFilePath = makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    QFile localPubKeyfile(strLocalPubKeyFilePath);
    if (!localPubKeyfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug() << "cant't open local public key file!";
        return false;
    }
    QString strLocalPubKey(localPubKeyfile.readAll());
    localPubKeyfile.close();

    QString strNewPubKey = strLocalPubKey.insert(USER_KEY_INTERCEPT_INDEX, userKey);

    // 利用完整公钥解密密文，得到密码
    QString strRSACipherFilePath = makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    QFile rsaCipherfile(strRSACipherFilePath);
    if (!rsaCipherfile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug() << "cant't open rsa cipher file!";
        return false;
    }
    QString strRsaCipher(rsaCipherfile.readAll());
    rsaCipherfile.close();

    QString strNewPassword = rsam::publicKeyDecrypt(strRsaCipher, strNewPubKey);

    // 判断密码的正确性，如果密码正确，则用户密钥正确，否则用户密钥错误
    if (!checkPassword(strNewPassword, cipher)) {
        qDebug() << "user key error!";
        return false;
    }

    return true;
}

QString OperatorCenter::getUserKey()
{
    return m_strUserKey;
}

bool OperatorCenter::getPasswordHint(QString &passwordHint)
{
    QString strPasswordHintFilePath = makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    if (!passwordHintFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        qDebug() << "open password hint file failure";
        return false;
    }
    passwordHint = QString(passwordHintFile.readAll());
    passwordHintFile.close();

    return true;
}

bool OperatorCenter::createQRCode(const QString &srcStr, int width, int height, QPixmap &pix)
{
    if (width < 1 || height < 1) {
        qDebug() << "QR code width or height error";
        return false;
    }

    QRcode *qrcode = QRcode_encodeString(srcStr.toStdString().c_str(), 2, QR_ECLEVEL_Q, QR_MODE_8, 1);
    // 二维码图片大小
    qint32 temp_width = width;
    qint32 temp_height = height;

    // 二维码的宽度（也是高度，正方向）
    qint32 qrcode_width = qrcode->width > 0 ? qrcode->width : 1;

    // 二维码图片的缩放比例
    double scale_x = double(temp_width) / double(qrcode_width);
    double scale_y = double(temp_height) / double(qrcode_width);

    // 生成二维码图片
    QImage mainimg = QImage(temp_width, temp_height, QImage::Format_ARGB32);
    QPainter painter(&mainimg);

    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, temp_width, temp_height);

    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    for (qint32 y = 0; y < qrcode_width; y++) {
        for (qint32 x = 0; x < qrcode_width; x++) {
            unsigned char b = qrcode->data[y * qrcode_width + x];
            if (b & 0x01) {
                QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }

    pix = QPixmap::fromImage(mainimg);

    if (qrcode)
        QRcode_free(qrcode);
    return true;
}

EN_VaultState OperatorCenter::vaultState()
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        return NotAvailable;
    }

    if (QFile::exists(makeVaultLocalPath(VAULT_ENCRYPY_DIR_NAME, CRYFS_CONFIG_FILE_NAME))) {
        QStorageInfo info(makeVaultLocalPath(VAULT_DECRYPT_DIR_NAME));
        if (info.isValid() && info.fileSystemType() == "fuse.cryfs") {
            return Unlocked;
        }
        return Encrypted;
    } else {
        return NotExisted;
    }
}

QString OperatorCenter::getSaltAndPasswordCipher()
{
    return m_strCryfsPassword;
}

void OperatorCenter::clearSaltAndPasswordCipher()
{
    m_strCryfsPassword.clear();
}

QString OperatorCenter::getEncryptDirPath()
{
    return makeVaultLocalPath(VAULT_ENCRYPY_DIR_NAME);
}

QString OperatorCenter::getdecryptDirPath()
{
    return makeVaultLocalPath(VAULT_DECRYPT_DIR_NAME);
}

QStringList OperatorCenter::getConfigFilePath()
{
    QStringList lstPath;

    lstPath << makeVaultLocalPath(PASSWORD_FILE_NAME);
    lstPath << makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    lstPath << makeVaultLocalPath(RSA_CIPHERTEXT_FILE_NAME);
    lstPath << makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);

    return lstPath;
}

QString OperatorCenter::autoGeneratePassword(int length)
{
    if (length < 3) return "";
    qsrand(uint(QTime(0, 0, 0).secsTo(QTime::currentTime())));

    QString strPassword("");

    QString strNum("0123456789");
    strPassword += strNum.at(qrand() % 10);

    QString strSpecialChar("`~!@#$%^&*");
    strPassword += strSpecialChar.at(qrand() % 10);

    QString strABC("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    strPassword += strABC.at(qrand() % 10);

    QString strAllChar = strNum + strSpecialChar + strABC;
    int nCount = length - 3;
    for (int i = 0; i < nCount; ++i) {
        strPassword += strAllChar.at(qrand() % 52);
    }
    return strPassword;
}

bool OperatorCenter::getRootPassword()
{
    // 判断当前是否是管理员登陆
    bool res = runCmd("id -un");   // file path is fixed. So write cmd direct
    if (res && m_standOutput.trimmed() == "root") {
        return true;
    }

    if (false == executeProcess("sudo whoami")) {
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
        qDebug() << "cmd is empty!";
        return -1;
    }

    if ((fp = popen(cmd, "r")) == nullptr) {
        perror("popen");
        qDebug() << QString("popen error: %s").arg(strerror(errno));
        return -1;
    } else {
        char buf[MAXLINE] = { '\0' };
        while (fgets(buf, sizeof(buf), fp)) {   // 获得每行输出
            QString strLineOutput(buf);
            if (strLineOutput.endsWith('\n'))
                strLineOutput.chop(1);
            lstShellOutput.push_back(strLineOutput);
        }

        int res;
        if ((res = pclose(fp)) == -1) {
            qDebug() << "close popen file pointer fp error!";
            return res;
        } else if (res == 0) {
            return res;
        } else {
            qDebug() << QString("popen res is : %1").arg(res);
            return res;
        }
    }
}
