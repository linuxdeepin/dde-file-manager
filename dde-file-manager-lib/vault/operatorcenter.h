#ifndef OPERATORCENTER_H
#define OPERATORCENTER_H

#include <QObject>

#include "vaultglobaldefine.h"


class OperatorCenter : public QObject
{
    Q_OBJECT
public:
    inline static OperatorCenter &getInstance(){
        static OperatorCenter instance;
        return instance;
    }
    ~OperatorCenter();

    /**
     * @brief createDirAndFile 创建保险箱目录及密钥文件
     * @return 是否成功
     */
    bool createDirAndFile();

    /**
     * @brief saveSaltAndClipher pbkdf2加密密码，并将盐和密文写入密码文件
     * @param password 密码
     * @return 是否成功
     */
    bool saveSaltAndClipher(const QString &password, const QString &passwordHint);

    /**
     * @brief createKey rsa生成密钥对，私钥加密密码，将密文写入文件，将一部分公钥写入文件（另一部分公钥提供给用户）
     * @param password 密码
     * @param bytes 提供给用户的那部分公钥的长度
     * @return 是否成功
     */
    bool createKey(const QString &password, int bytes);

    /**
     * @brief checkPassword 验证密码是否正确
     * @param password 密码
     * @param clipher 如果密码正确，返回加密后的密文，用于解锁保险箱
     * @return 是否正确
     */
    bool checkPassword(const QString &password, QString &clipher);

    /**
     * @brief checkUserKey 验证用户密钥是否正确
     * @param userKey 用户密钥
     * @param clipher 如果密钥正确，返回加密后的密文，用于解锁保险箱
     * @return 是否正确
     */
    bool checkUserKey(const QString &userKey, QString &clipher);

    /**
     * @brief getUserKey 获得用户密钥
     * @return 用户密钥
     */
    QString getUserKey();

    /**
     * @brief getPasswordHint 获取密码提示信息
     * @param passwordHint 返回提示信息
     * @return 是否成功
     */
    bool getPasswordHint(QString &passwordHint);

    /**
     * @brief createQRCode 生成二维码
     * @param srcStr 数据源
     * @param width 二维码图片宽度
     * @param height 二维码图片高度
     * @param pix   返回二维码图片
     * @return 是否成功
     */
    bool createQRCode(const QString &srcStr, int width, int height, QPixmap &pix);

    //! 获取保险箱状态
    EN_VaultState vaultState();

    //! 获取盐值及密码密文
    QString getSaltAndPasswordClipher();

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

signals:

public slots:

private:
    OperatorCenter(QObject *parent = nullptr);
    // 组织保险箱本地文件路径
    QString makeVaultLocalPath(const QString &before = "", const QString &behind = "");
    bool runCmd(const QString &cmd);
    bool executeProcess(const QString &cmd);

private:
    QString                 m_strUserKey;
    QString                 standOutput_;
};

#endif // OPERATORCENTER_H
