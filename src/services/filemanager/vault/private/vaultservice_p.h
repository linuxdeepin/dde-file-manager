#ifndef VAULTSERVICE_P_H
#define VAULTSERVICE_P_H
#include "dfm_filemanager_service_global.h"
#include "vault/vaulterrorcode.h"

#include <QObject>
#include <QSharedPointer>

DSB_FM_BEGIN_NAMESPACE

class VaultHandle;
class VaultService;
class VaultServicePrivate : public QObject
{
    Q_OBJECT

    friend class VaultService;

private:
    explicit VaultServicePrivate(QObject *parent = nullptr);

public:
    virtual ~VaultServicePrivate() override;

signals:

    /*!
     * \brief                       创建保险箱信号
     * \param[in] lockBaseDir:      保险箱加密文件夹
     * \param[in] unlockFileDir:    保险箱解密文件夹
     * \param[in] DSecureString:    保险箱密码
     */
    void sigCreateVault(QString encryptBaseDir, QString decryptFileDir, QString DSecureString, EncryptType type = EncryptType::AES_256_GCM, int blockSize = 32768);

    /*!
     * \brief                       解锁保险箱信号
     * \param[in] lockBaseDir:      保险箱加密文件夹
     * \param[in] unlockFileDir:    保险箱解密文件夹
     * \param[in] DSecureString:    保险箱密码
     */
    void sigUnlockVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString);

    /*!
     * \brief                        加锁保险箱信号
     * \param[in] unlockFileDir:     保险箱解密文件夹
     */
    void sigLockVault(QString unlockFileDir);

signals:
    /*!
     * \brief                错误输出
     * \param[in] error:     错误信息
     */
    void signalReadErrorPrivate(QString error);

    /*!
     * \brief                标准输出
     * \param[in] msg:       输出信息
     */
    void signalReadOutputPrivate(QString msg);

    /*!
     * \brief                   创建保险箱是否成功的信号
     * \param[in] state:        返回ErrorCode枚举值
     */
    void signalCreateVaultPrivate(int state);

    /*!
     * \brief                   解锁保险箱是否成功的信号
     * \param[in] state:        返回ErrorCode枚举值
     */
    void signalUnlockVaultPrivate(int state);

    /*!
     * \brief                   加锁保险箱是否成功的信号
     * \param[in] state         返回ErrorCode枚举值
     */
    void signalLockVaultPrivate(int state);

private:
    /*!
     * \brief  保险箱处理对象
     */
    QSharedPointer<VaultHandle> vaultHandle;
};
DSB_FM_END_NAMESPACE
#endif   // VAULTSERVICE_P_H
