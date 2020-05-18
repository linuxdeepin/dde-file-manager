#ifndef VAULTLOCKMANAGER_H
#define VAULTLOCKMANAGER_H

#include "dabstractfilecontroller.h"

#include "dfmglobal.h"

#include <QTimer>

class VaultInterface;
class VaultLockManager : public QObject
{
    Q_OBJECT

public:

    enum AutoLockState {
        Never = 0,
        FiveMinutes = 5,
        TenMinutes = 10,
        TwentyMinutes = 20
    };

    static VaultLockManager &getInstance();

    /**
     * @brief autoLockState    自动上锁状态
     * @return                 返回状态值
     */
    AutoLockState autoLockState() const;

    /**
     * @brief autoLock    设置自动锁状态
     * @param lockState   状态值
     * @return
     */
    bool autoLock(AutoLockState lockState);

    /**
     * @brief isValid 保险箱服务是否开启
     * @return
     */
    bool isValid() const;

    /**
     * @brief refreshAccessTime 刷新保险箱访问时间
     */
    void refreshAccessTime();

protected slots:
    /**
     * @brief processAutoLock 处理自动加锁
     */
    void processAutoLock();

    /**
     * @brief slotLockVault 加锁状态槽函数
     */
    void slotLockVault(QString msg);

    /**
     * @brief slotUnlockVault 解锁状态槽函数
     */
    void slotUnlockVault();

private:
    explicit VaultLockManager(QObject *parent = nullptr);
    ~VaultLockManager();

    /**
     * @brief loadConfig 加载配置文件
     */
    void loadConfig();

    /**
     * @brief setRefreshTime 刷新保险柜最新计时
     * @param time 时间
     * @return
     */
    void dbusSetRefreshTime(quint64 time);

    /**
     * @brief getLastestTime 获取最新计时
     * @return
     */
    quint64 dbusGetLastestTime() const;

private:
    VaultInterface* m_vaultInterface = nullptr; // 交互接口

    qulonglong m_lastestTime; // 最新计时

    AutoLockState m_autoLockState; // 自动锁状态

    DAbstractFileInfoPointer m_rootFileInfo; // 根目录文件信息

    QTimer m_alarmClock; // 自动锁计时器

    bool m_isCacheTimeReloaded; // 访问时间是否已经重新加载
};

#endif // VAULTLOCKMANAGER_H
