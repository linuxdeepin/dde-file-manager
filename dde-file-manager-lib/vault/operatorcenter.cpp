#include "operatorcenter.h"
#include "openssl/pbkdf2.h"
#include "openssl/rsam.h"
#include "qrencode/qrencode.h"

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
    : QObject(parent)
{
    m_strUserKey.clear();
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
    QProcess process_;
    int mescs = 10000;
    if(cmd.startsWith(ROOT_PROXY)){
        mescs = -1;
    }
    process_.start(cmd);

    bool res = process_.waitForFinished(mescs);
    standOutput_ = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if(cmd.startsWith(ROOT_PROXY) && (exitCode == 127 || exitCode == 126)){
        QString strOut = "Run \'" + cmd + "\' fauled: Password Error! " + QString::number(exitCode) + "\n";
        qDebug() << strOut;
        return false;
    }

    if(res == false){
        QString strOut = "Run \'" + cmd + "\' failed\n";
        qDebug() << strOut;
    }

    return res;
}

bool OperatorCenter::executeProcess(const QString &cmd)
{
    if ( false == cmd.startsWith("sudo") ) {
        return runCmd(cmd);
    }

    runCmd("id -un");
    if (standOutput_.trimmed() == "root") {
        return runCmd(cmd);
    }

    QString newCmd = QString(ROOT_PROXY) + " \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd);
}

OperatorCenter::~OperatorCenter()
{
}

bool OperatorCenter::createDirAndFile()
{
    // 创建配置文件目录
    QString strConfigDir = makeVaultLocalPath();
    QDir configDir(strConfigDir);
    if(!configDir.exists()){
        bool ok = configDir.mkpath(strConfigDir);
        if(!ok){
            qDebug() << "create config dir failure!";
            return false;
        }
    }

    // 创建密码文件
    QString strPasswordFile = makeVaultLocalPath(PASSWORD_FILE_NAME);
    QFile passwordFile(strPasswordFile);
    if(!passwordFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "create password file failure!";
        return false;
    }
    passwordFile.close();

    // 创建存放rsa公钥的文件
    QString strPriKeyFile = makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    QFile prikeyFile(strPriKeyFile);
    if(!prikeyFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "create rsa private key file failure!";
        return false;
    }
    prikeyFile.close();

    // 创建存放rsa公钥加密后密文的文件
    QString strRsaClipher = makeVaultLocalPath(RSA_CLIPHERTEXT_FILE_NAME);
    QFile rsaClipherFile(strRsaClipher);
    if(!rsaClipherFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "create rsa clipher file failure!";
        return false;
    }
    rsaClipherFile.close();

    // 创建密码提示信息文件
    QString strPasswordHintFilePath = makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    if(!passwordHintFile.open(QIODevice::WriteOnly | QIODevice:: Append)){
        qDebug() << "create password hint file failure!";
        return false;
    }
    passwordHintFile.close();

    return true;
}

bool OperatorCenter::saveSaltAndClipher(const QString &password, const QString &passwordHint)
{
    // 加密密码，并将盐和密文写入密码文件
    // 随机盐
    QString strRandomSalt = pbkdf2::createRandomSalt(RANDOM_SALT_LENGTH);
    // 密文
    QString strClipherText = pbkdf2::pbkdf2EncrypyPassword(password, strRandomSalt, ITERATION, PASSWORD_CLIPHER_LENGTH);
    // 写入文件
    QString strSaltAndClipherText = strRandomSalt + strClipherText;
    QString strPasswordFile = makeVaultLocalPath(PASSWORD_FILE_NAME);
    QFile passwordFile(strPasswordFile);
    if(!passwordFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug() << "write cliphertext failure!";
        return false;
    }
    QTextStream out(&passwordFile);
    out << strSaltAndClipherText;
    passwordFile.close();

    // 保存密码提示信息
    QString strPasswordHintFilePath = makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);
    QFile passwordHintFile(strPasswordHintFilePath);
    if(!passwordHintFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug() << "write password hint failure";
        return false;
    }
    QTextStream out2(&passwordHintFile);
    out2 << passwordHint;
    passwordHintFile.close();

    return true;
}

bool OperatorCenter::createKey(const QString &password, int bytes)
{
    // 清空上次的用户密钥
    m_strUserKey.clear();

    // 创建密钥对
    QString strPriKey("");
    QString strPubKey("");
    rsam::createRsaKey(strPubKey, strPriKey);

    // 私钥加密
    QString strClipher = rsam::rsa_pri_encrypt_base64(password, strPriKey);

    // 将公钥分成两部分（一部分用于保存到本地，一部分生成二维码，提供给用户）
    QString strSaveToLocal("");
    if(strPubKey.length() < 2 * USER_KEY_INTERCEPT_INDEX + bytes){
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
    if(!publicFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug() << "open public key file failure!";
        return false;
    }
    QTextStream out(&publicFile);
    out << strSaveToLocal;
    publicFile.close();

    // 保存密文
    QString strClipherFilePath = makeVaultLocalPath(RSA_CLIPHERTEXT_FILE_NAME);
    QFile clipherFile(strClipherFilePath);
    if(!clipherFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug() << "open rsa clipher file failure!";
        return false;
    }
    QTextStream out2(&clipherFile);
    out2 << strClipher;
    clipherFile.close();

    return true;
}

bool OperatorCenter::checkPassword(const QString &password, QString &clipher)
{
    // 获得本地盐及密文
    QString strfilePath = makeVaultLocalPath(PASSWORD_FILE_NAME);
    QFile file(strfilePath);
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
        qDebug() << "open pbkdf2clipher file failure!";
        return false;
    }
    QString strSaltAndClipher = QString(file.readAll());
    file.close();
    QString strSalt = strSaltAndClipher.mid(0, RANDOM_SALT_LENGTH);
    QString strClipher = strSaltAndClipher.mid(RANDOM_SALT_LENGTH);

    // pbkdf2加密密码,获得密文
    QString strNewClipher = pbkdf2::pbkdf2EncrypyPassword(password, strSalt, ITERATION, PASSWORD_CLIPHER_LENGTH);
    QString strNewSaltAndClipher = strSalt + strNewClipher;
    if(strNewSaltAndClipher != strSaltAndClipher){
        qDebug() << "password error!";
        return  false;
    }

    clipher = strNewSaltAndClipher;

    return true;
}

bool OperatorCenter::checkUserKey(const QString &userKey, QString &clipher)
{
    if(userKey.length() != USER_KEY_LENGTH){
        qDebug() << "user key length error!";
        return  false;
    }

    // 结合本地公钥和用户密钥，还原完整公钥
    QString strLocalPubKeyFilePath = makeVaultLocalPath(RSA_PUB_KEY_FILE_NAME);
    QFile localPubKeyfile(strLocalPubKeyFilePath);
    if(!localPubKeyfile.open(QIODevice::Text | QIODevice::ReadOnly)){
        qDebug() << "cant't open local public key file!";
        return false;
    }
    QString strLocalPubKey(localPubKeyfile.readAll());
    localPubKeyfile.close();

    QString strNewPubKey = strLocalPubKey.insert(USER_KEY_INTERCEPT_INDEX, userKey);

    // 利用完整公钥解密密文，得到密码
    QString strRSAClipherFilePath = makeVaultLocalPath(RSA_CLIPHERTEXT_FILE_NAME);
    QFile rsaClipherfile(strRSAClipherFilePath);
    if(!rsaClipherfile.open(QIODevice::Text | QIODevice::ReadOnly)){
        qDebug() << "cant't open rsa clipher file!";
        return false;
    }
    QString strRsaClipher(rsaClipherfile.readAll());
    rsaClipherfile.close();

    QString strNewPassword = rsam::rsa_pub_decrypt_base64(strRsaClipher, strNewPubKey);

    // 判断密码的正确性，如果密码正确，则用户密钥正确，否则用户密钥错误
    if(!checkPassword(strNewPassword, clipher)){
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
    if(!passwordHintFile.open(QIODevice::Text | QIODevice::ReadOnly)){
        qDebug() << "open password hint file failure";
        return false;
    }
    passwordHint = QString(passwordHintFile.readAll());
    passwordHintFile.close();

    return true;
}

bool OperatorCenter::createQRCode(const QString &srcStr, int width, int height, QPixmap &pix)
{
    if(width < 1 || height < 1){
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
    for(qint32 y = 0; y < qrcode_width; y++){
        for(qint32 x = 0; x < qrcode_width; x++){
            unsigned char b = qrcode->data[y * qrcode_width + x];
            if(b & 0x01){
                QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }

    pix = QPixmap::fromImage(mainimg);

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

QString OperatorCenter::getSaltAndPasswordClipher()
{
    QString strfilePath = makeVaultLocalPath(PASSWORD_FILE_NAME);
    QFile file(strfilePath);
    if(!file.open(QIODevice::Text | QIODevice::ReadOnly)){
        qDebug() << "open pbkdf2clipher file failure!";
        return "";
    }
    QString strSaltAndClipher = QString(file.readAll());
    file.close();
    return strSaltAndClipher;
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
    lstPath << makeVaultLocalPath(RSA_CLIPHERTEXT_FILE_NAME);
    lstPath << makeVaultLocalPath(PASSWORD_HINT_FILE_NAME);

    return lstPath;
}

QString OperatorCenter::autoGeneratePassword(int length)
{
    if(length < 3) return "";
    qsrand(uint(QTime(0, 0, 0).secsTo(QTime::currentTime())));

    QString strPassword("");

    QString strNum("0123456789");
    strPassword += strNum.at(qrand()%10);

    QString strSpecialChar("`~!@#$%^&*");
    strPassword += strSpecialChar.at(qrand()%10);

    QString strABC("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    strPassword += strABC.at(qrand()%10);

    QString strAllChar = strNum + strSpecialChar + strABC;
    int nCount = length - 3;
    for(int i = 0; i < nCount; ++i)
    {
        strPassword += strAllChar.at(qrand()%52);
    }
    return strPassword;
}

bool OperatorCenter::getRootPassword()
{
    // 判断当前是否是管理员登陆
    bool res = runCmd("id -un");  // file path is fixed. So write cmd direct
    if ( res == true && standOutput_.trimmed() == "root" ) {
        return true;
    }

    if ( false == executeProcess("sudo whoami")) {
        return false;
    }

    return true;
}

int OperatorCenter::executionShellCommand(const QString &strCmd, QStringList &lstShellOutput)
{
    FILE *fp;
    int res;
    char buf[MAXLINE] = {'\0'};

    std::string sCmd = strCmd.toStdString();
    const char* cmd = sCmd.c_str();

    // 命令为空
    if(strCmd.isEmpty()) {
        qDebug() << "cmd is empty!";
        return -1;
    }

    if((fp = popen(cmd, "r")) == nullptr){
        perror("popen");
        qDebug() << QString("popen error: %s").arg(strerror(errno));
        return -1;
    }else{
        while(fgets(buf, sizeof(buf), fp)){ // 获得每行输出
            QString strLineOutput(buf);
            if(strLineOutput.endsWith('\n'))
                strLineOutput.chop(1);
            lstShellOutput.push_back(strLineOutput);
        }

        if((res = pclose(fp)) == -1){
            qDebug() << "close popen file pointer fp error!";
            return res;
        }else if(res == 0){
            return res;
        }else{
            qDebug() << QString("popen res is : %1").arg(res);
            return res;
        }
    }
}

