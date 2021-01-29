/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-05-08

 */

#ifndef VAULTCONTROLLER_H
#define VAULTCONTROLLER_H

#include "dabstractfilecontroller.h"

#include <DSecureString>

DFM_BEGIN_NAMESPACE
class DFileStatisticsJob;
DFM_END_NAMESPACE

DCORE_USE_NAMESPACE

class VaultControllerPrivate;
class VaultController : public DAbstractFileController
{
    Q_OBJECT
public:
    enum VaultState {
        NotExisted,
        Encrypted,
        Unlocked,
        UnderProcess,
        Broken,
        NotAvailable
    };

    struct FileBaseInfo {
        bool isExist = false;
        bool isFile;
        bool isDir;
        bool isWritable = true;
        bool isSymLink = false;
    };

    explicit VaultController(QObject *parent = nullptr);
public:

    static VaultController *ins();

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFiles(const QSharedPointer<DFMOpenFilesEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    /**
     * @brief shareFolder 设置文件夹共享
     * @param event       共享的信息事件
     * @return            是否共享成功
     */
    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const override;

    /**
     * @brief unShareFolder 取消文件夹共享
     * @param event         关闭共享信息事件
     * @return              是否取消成功
     */
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const override;

    /**
     * @brief openInTerminal  右键菜单打开当前路径终端
     * @param event           打开终端信息事件
     * @return                是否打开终端成功
     */
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const override;

    /**
     * @brief addToBookmark   添加当前文件夹书签
     * @param event           添加书签信息事件
     * @return                是否添加成功
     */
    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;

    /**
     * @brief removeBookmark  移除当前文件夹书签
     * @param event           移除书签信息事件
     * @return                是否移除成功
     */
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;

    /**
     * @brief createSymlink  创建快捷方式
     * @param event          创建快捷方式信息事件
     * @return               是否创建成功
     */
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;

    /**
     * @brief setFileTags    设置文件标记信息
     * @param event          设置文件标记信息事件
     * @return               是否标记成功
     */
    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;

    /**
     * @brief removeTagsOfFile  移除文件标记信息
     * @param event             移除文件标记信息事件
     * @return                  是否移除文件标记
     */
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;

    /**
     * @brief getTagsThroughFiles   获取文件标记信息
     * @param event                 获取文件标记信息事件
     * @return                      标记信息列表
     */
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;

    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;

    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const override;

    void updateFileInfo(const DUrlList &fileUrls);
    FileBaseInfo getFileInfo(const DUrl &fileUrl);
    static DUrl makeVaultUrl(QString path = "", QString host = "");
    static DUrl localUrlToVault(const DUrl &vaultUrl);
    static DUrl localToVault(QString localPath);
    static QString vaultToLocal(const DUrl &vaultUrl);
    static DUrl vaultToLocalUrl(const DUrl &vaultUrl);
    static DUrlList vaultToLocalUrls(DUrlList vaultUrls);

    static QString pathToVirtualPath(QString path);

    static DUrl urlToVirtualUrl(QString path);

    /**
     * @brief isDeleteFiles 判断保险箱当前是否正在删除文件
     * @return true/false
     */
    static bool isBigFileDeleting();

    /**
     * @brief isVaultFile  是否为保险箱中的文件
     * @param path         文件路径
     * @return
     */
    static bool isVaultFile(QString path);

    /**
     * @brief getPermissions 获取文件的权限信息
     * @param filePath       文件路径
     * @return
     */
    static QFileDevice::Permissions getPermissions(QString filePath);

    /**
     * @brief state         获取当前保险箱状态
     * @param lockBaseDir   保险箱加密文件夹
     * @return              返回VaultState枚举值
     */
    VaultState state(QString lockBaseDir = "");

    /**
     * @brief VaultFileInfo::isRootDirectory 是否为保险箱根目录
     * @return
     */
    static bool isRootDirectory(QString path);

    /**
     * @brief getErrorInfo 根据错误码获取错误信息
     * @param state 错误码
     * @return
     */
    static QString getErrorInfo(int state);

    /**
     * @brief toInternalPath 外部路径转内部路径
     * @param external
     * @return
     */
    static QString toInternalPath(const QString &externalPath);

    /**
     * @brief toExternalPath 转换成外部路径
     * @param internalPath
     * @return
     */
    static QString toExternalPath(const QString &internalPath);

    /**
    * @brief localPathToVirtualPath 本地路径转虚拟路径
    * @param localPath
    * @return
    */
    static QString localPathToVirtualPath(const QString &localPath);

    /**
     * @brief virtualPathToLocalPath 虚拟路径转本地路径
     * @param virtualPath
     * @return
     */
    static QString virtualPathToLocalPath(const QString &virtualPath);

    /**
     * @brief totalsize 保险箱大小
     * @return
     */
    qint64 totalsize() const;

    // 获得保险箱状态
    inline VaultState getVaultState() const
    {
        return m_enVaultState;
    }
    // 设置保险箱状态
    inline void setVaultState(const VaultState &state)
    {
        m_enVaultState = state;
    }

    /**
     * @brief setBigFileIsDeleting Record big file deleting state.
     * Avoid block while mutl-file deleted.
     * @param isDeleting
     */
    void setBigFileIsDeleting(bool const isDeleting);

public slots:

    void updateFolderSizeLabel(const qint64 size) noexcept;

    /**
     * @brief createVault       创建保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     * @param passWord          保险箱密码

     */
    void createVault(const DSecureString &password, QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief unlockVault       解锁保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     * @param passWord          保险箱密码
     */
    void unlockVault(const DSecureString &password, QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief lockVault         加锁保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     */
    void lockVault(QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief makeVaultLocalPath    创建本地路径
     * @param path                  子目录(文件或文件路径)
     * @param base                  父目录
     * @return                      返回新路径
     */
    static QString makeVaultLocalPath(QString path = "", QString base = "");

    /**
     * @brief vaultLockPath   返回默认保险箱加密文件夹路径，如路径是外部传入暂时无法获取
     * @return                默认保险箱加密文件夹路径
     */
    static QString vaultLockPath();

    /**
     * @brief vaultLockPath   返回默认保险箱解密文件夹路径，如路径是外部传入暂时无法获取
     * @return                默认保险箱解密文件夹路径
     */
    static QString vaultUnlockPath();

    /**
     * @brief refreshTotalSize 刷新保险箱大小
     */
    void refreshTotalSize();

    /**
     * @brief refreshTotalSize 刷新保险箱大小完成
     */
    void onFinishCalcSize();

    /**
     * @brief taskPaused 处理任务暂停信号
     * @param src 任务源路径
     * @param dst 任务目的路径
     */
    void taskPaused(const DUrlList &src, const DUrl &dst);

private slots:
    // 创建保险箱，执行该槽函数,通知保险箱创建成功与否，并更新保险箱的状态
    void slotCreateVault(int state);
    void slotUnlockVault(int state);
    void slotLockVault(int state);

signals:
    /**
     * @brief readError 错误输出
     * @param error     错误信息
     */
    void signalReadError(QString error);

    /**
     * @brief signalReadOutput  标准输出
     * @param msg               输出信息
     */
    void signalReadOutput(QString msg);

    /**
     * @brief signalCreateVault 创建保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalCreateVault(int state);

    /**
     * @brief singalUnlockVault 解锁保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalUnlockVault(int state);

    /**
     * @brief signalLockVault   加锁保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalLockVault(int state);

signals:
    /**
    * @brief 下列信号为本类内部使用，请勿外用
    */

    /**
     * @brief sigCreateVault    创建保险箱信号
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void sigCreateVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief sigUnlockVault    解锁保险箱信号
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void sigUnlockVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief sigLockVault      加锁保险箱信号
     * @param unlockFileDir     保险箱解密文件夹
     */
    void sigLockVault(QString unlockFileDir);

private:
    QMap<DUrl, FileBaseInfo> m_mapVaultFileInfo;

    VaultControllerPrivate *d_ptr;

    static VaultController *cryfs;

    qint64 m_totalSize = 0;

    static bool m_isBigFileDeleting;

    // 记录保险箱状态
    VaultState m_enVaultState{NotExisted};

    DFM_NAMESPACE::DFileStatisticsJob *m_sizeWorker{ nullptr };

    // 计算当前保险箱大小是否需要刷新
    bool m_bNeedRefreshSize = false;

    Q_DECLARE_PRIVATE(VaultController)
};

#endif
