#ifndef DBUSCONTROLLER_H
#define DBUSCONTROLLER_H

#include <QtCore>
#include <QtDBus>

#include "dbusinterface/dbustype.h"

class MonitorManagerInterface;
class FileMonitorInstanceInterface;
class ClipboardInterface;
class WatcherInstanceInterface;
class FileInfoInterface;
class DesktopDaemonInterface;
class FileOperationsInterface;
class CreateDirJobInterface;
class CreateFileJobInterface;
class CreateFileFromTemplateJobInterface;
class AppearanceDaemonInterface;
class DisplayInterface;
class PinyinInterface;
class DBusClientManager;
class FileMonitor;
class QTimer;
class DBusDockSetting;
class AppController;

#define FileMonitor_service "com.deepin.filemanager.Backend.Monitor"
#define FileMonitor_path "/com/deepin/filemanager/Backend/MonitorManager"
#define FileOperations_path "/com/deepin/filemanager/Backend/Operations"
#define TrashMonitor_path "/com/deepin/filemanager/Backend/Monitor/TrashMonitor"
#define Clipboard_path "/com/deepin/filemanager/Backend/Clipboard"

#define FileInfo_service "com.deepin.filemanager.Backend.FileInfo"
#define FileInfo_path "/com/deepin/filemanager/Backend/FileInfo"

#define DesktopDaemon_service "com.deepin.dde.daemon.Desktop"
#define DesktopDaemon_path "/com/deepin/dde/daemon/Desktop"

#define Appearance_service "com.deepin.daemon.Appearance"
#define Appearance_path "/com/deepin/daemon/Appearance"

#define Watcher_service "com.deepin.filemanager.Backend.Watcher"

#define Pinyin_service "com.deepin.api.Pinyin"
#define Pinyin_path "/com/deepin/api/Pinyin"

typedef QSharedPointer<FileMonitorInstanceInterface> FileMonitorInstanceInterfacePointer;


class DBusController : public QObject
{
    Q_OBJECT

    enum G_FILE_MONITOR_EVENT{
        G_FILE_MONITOR_EVENT_CHANGED,
        G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT,
        G_FILE_MONITOR_EVENT_DELETED,
        G_FILE_MONITOR_EVENT_CREATED,
        G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED,
        G_FILE_MONITOR_EVENT_PRE_UNMOUNT,
        G_FILE_MONITOR_EVENT_UNMOUNTED,
        G_FILE_MONITOR_EVENT_MOVED
    };

    enum G_FILE_MONITOR_FLAG{
        G_FILE_MONITOR_NONE = 0x0,
        G_FILE_MONITOR_WATCH_MOUNTS = 0x01,
        G_FILE_MONITOR_SEND_MOVED = 0x02,
        G_FILE_MONITOR_WATCH_HARD_LINKS = 0x04
    };

public:
    DBusController(QObject *parent = 0);
    ~DBusController();
    void init();
    void initConnect();

    static int RequestThumailMaxCount;

    FileOperationsInterface* getFileOperationsInterface();
    FileInfoInterface* getFileInfoInterface();

    QMap<QString, FileItemInfoMap> getAppGroups();

    FileItemInfoMap getDesktopItemInfoMap();
    void asyncRenameDesktopItemByUrl(QString url);
    void asyncCreateDesktopItemByUrl(QString url);

    DesktopDaemonInterface* getDesktopDaemonInterface();
    DisplayInterface* getDisplayInterface();

    void updateDesktopItemInfoMap(FileItemInfo desktopItemInfo);
    void updateDesktopItemInfoMap_moved(FileItemInfo desktopItemInfo);
    void removeDesktopItemInfoByUrl(QString url);

    int getDockMode();

signals:

public slots:
    void loadDesktopSettings();
    void loadDesktopItems();

    void requestIconByUrl(QString url, uint size = 48);
    void requestThumbnail(QString url, uint size = 48);

    void handleFileCreated(const QString& path);
    void handleFileDeleted(const QString& path);
    void handleFileMovedIn(const QString& path);
    void handleFileMovedOut(const QString& path);
    void handleFileRenamed(const QString& oldPath, const QString& newPath);
    void handleFileMetaDataChanged(const QString& path);

    void asyncRequestDesktopItems();
    void asyncRequestComputerIcon();
    void asyncRequestComputerIconFinihsed(QDBusPendingCallWatcher *call);
    void asyncRequestTrashIcon();
    void asyncRequestTrashIconFinished(QDBusPendingCallWatcher *call);

    void asyncRequestDesktopItemsFinished(QDBusPendingCallWatcher* call);
    void asyncRenameDesktopItemByUrlFinished(QDBusPendingCallWatcher* call);
    void asyncCreateDesktopItemByUrlFinished(QDBusPendingCallWatcher* call);

    void openFiles(QStringList files, IntList intFlags);
    void openFiles(FileItemInfo destinationDesktopItemInfo, QStringList urls);
    void openFiles(FileItemInfo destinationDesktopItemInfo, QList<FileItemInfo> desktopItemInfos);
    void openFile(FileItemInfo desktopItemInfo);

    /*create file*/
    void createDirectory();
    void createDirectoryFinished(QString dirName, QString error);
    void createFile();
    void createFileFinished(QString filename, QString error);
    void createFileFromTemplate(QString templatefile);
    void createFileFromTemplateFinished(QString filename, QString error);
    void sortByKey(QString key);

    /*copy files*/
    void copyFiles(QStringList urls);

    /*cut files*/
    void cutFiles(QStringList urls);

    /*app group*/
    void requestCreatingAppGroup(QStringList urls);
    void getAppGroupItemsByUrl(QString group_url);
    void createAppGroup(QString group_url, QStringList urls);
    void mergeIntoAppGroup(QStringList urls, QString group_url);

    /*paste files*/
    void pasteFilesToDesktop();
    void pasteFiles(QString action, QStringList files, QString destination);

    /*delay get thumbnail*/
    void delayGetThumbnail();

    /*handle theme changed*/
    void handelIconThemeChanged();

    /*request thumbnail to update icon for get icon error by tmp file*/
    void refreshThumail(QString url, uint size=48);

    /*request pinyin convert*/
    void convertNameToPinyin();
    void sortPingyinEnglish();

    /*delay rename event from inotify*/
    void delayHandleRenameEvent();
private:
    FileMonitor* m_fileMonitor = NULL;
    FileMonitorInstanceInterface* m_desktopMonitorInterface = NULL;
    ClipboardInterface* m_clipboardInterface = NULL;
    QMap<QString, FileMonitorInstanceInterfacePointer> m_appGroupMonitorInterfacePointers;
    FileInfoInterface* m_fileInfoInterface = NULL;
    DesktopDaemonInterface* m_desktopDaemonInterface = NULL;
    FileOperationsInterface* m_fileOperationsInterface = NULL;

    CreateDirJobInterface* m_createDirJobInterface = NULL;
    CreateFileJobInterface* m_createFileJobInterface = NULL;
    CreateFileFromTemplateJobInterface* m_createFileFromTemplateJobInterface = NULL;
    DisplayInterface* m_displayInterface;
    PinyinInterface* m_pinyinInterface;
    DBusDockSetting* m_dockSettingInterface = NULL;
    AppearanceDaemonInterface* m_appearanceInterface;
    DBusClientManager* m_dockClientManagerInterface;
    AppController* m_appController;

    FileItemInfoMap m_desktopItemInfoMap;
    QMap<QString, FileItemInfoMap> m_appGroups;
    QList<FileItemInfo> m_sortedPinyinInfos;

    QString m_itemShoudBeMoved;

    QTimer* m_thumbnailTimer;
    QTimer* m_pinyinTimer;
    QString m_currentThumbnail;
    QStringList m_thumbnails;

    bool m_requestFinished = false;
    int m_thumbnailCount = 0;

    QString m_renameOldPath="";
    QString m_renameNewPath="";

    Q_DISABLE_COPY(DBusController)


};

#endif // DBUSCONTROLLER_H
