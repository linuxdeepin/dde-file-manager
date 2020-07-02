#ifndef VAULTGLOBALDEFINE_H
#define VAULTGLOBALDEFINE_H

#include <QString>

#define CONFIG_DIR_NAME                 "vault"
#define VAULT_DECRYPT_DIR_NAME          "vault_unlocked"
#define VAULT_ENCRYPY_DIR_NAME          "vault_encrypted"

#define CRYFS_CONFIG_FILE_NAME          "cryfs.config"

#define PASSWORD_FILE_NAME              "pbkdf2clipher"
#define RSA_PUB_KEY_FILE_NAME           "rsapubkey"
#define RSA_CLIPHERTEXT_FILE_NAME       "rsaclipher"
#define PASSWORD_HINT_FILE_NAME         "passwordHint"


#define RANDOM_SALT_LENGTH              10              // 随机盐的字节数
#define ITERATION                       1024            // pbkdf2迭代次数
#define PASSWORD_CLIPHER_LENGTH         50              // 密码密文长度
#define USER_KEY_LENGTH                 32              // 用户密钥长度
#define USER_KEY_INTERCEPT_INDEX        50              // 用户密钥从公钥中截取的起始点索引

#define ROOT_PROXY                      "pkexec deepin-vault-authenticateProxy"

#define MAXLINE                         1024            // shell命令输出每行最大的字符个数

#define VAULT_BASE_PATH (QDir::homePath() + QString("/.local/share/applications"))  //! 获取保险箱创建的目录地址

#define GET_COMPRESSOR_PID_SHELL(x)     QString("ps -xo pid,cmd | grep /usr/bin/deepin-compressor | grep ")+x+QString(" | grep -v grep | awk '{print $1}'")

enum EN_VaultState {
    NotExisted = 0,
    Encrypted,
    Unlocked,
    UnderProcess,
    Broken,
    NotAvailable
};

#endif // VAULTGLOBALDEFINE_H
