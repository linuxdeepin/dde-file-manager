#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include <QObject>

class DFileInfo;
class DFMEvent;
class DFMUrlBaseEvent;
class DFMUrlListBaseEvent;
class DAbstractFileInfo;
class DeviceInfo;
class DUrl;

typedef QList<DUrl> DUrlList;



class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0);

signals:
    /* emit usershare status changed*/
    void userShareCountChanged(const int count);

    /* request close current tab*/
    void requestCloseCurrentTab(quint64 winId);

    /* request fetch network nodes*/

    void requestFetchNetworks(const DFMUrlBaseEvent &event);

    /*request fetch network nodes successfully*/
    void fetchNetworksSuccessed(const DFMUrlBaseEvent &event);

    /*request mount smb*/
    void requestSMBMount(const DFMUrlBaseEvent &event);

    /*request choose smb mounted file*/
    void requestChooseSmbMountedFile(const DFMUrlBaseEvent &event);

    /* current display url changed*/
    void currentUrlChanged(const DFMUrlBaseEvent &event);

    /* file icon*/
    void requestIcon(const DUrl &url) const;
    void iconChanged(const DUrl &url, const QIcon &icon) const;

    /*add copy/move/delete job to taskdialog when copy/move/delete job created*/
    void jobAdded(const QMap<QString, QString>& jobDetail);

    /*remove copy/move/delete job to taskdialog when copy/move/delete job finished*/
    void jobRemoved(const QMap<QString, QString>& jobDetail);

    /*update copy/move/delete job taskdialog ui*/
    void jobDataUpdated(const QMap<QString, QString>& jobDetail,
                           const QMap<QString, QString>& data);

    /*abort copy/move/delete job taskdialog from ui*/
    void abortTask(const QMap<QString, QString>& jobDetail);

    /*copy/move job conflict dialog show */
    void conflictDialogShowed(const QMap<QString, QString>& jobDetail);

    /*request rename to DFileView*/
    void requestRename(const DFMUrlBaseEvent &event);

    /* view select all */
    void requestViewSelectAll(int windowId);

    /* bookmark remove */
    void requestBookmarkRemove(const DFMUrlBaseEvent &event);

    /* bookmark add */
    void requestBookmarkAdd(const QString &name, const DFMUrlBaseEvent &event);

    /* bookmark move */
    void requestBookmarkMove(int from, int to, const DFMEvent &event);

    /*requst open new window by given url*/
    void requestOpenNewWindowByUrl(const DUrl& url, bool isAlwaysOpen);

    /*request active window*/
    void requestActiveWindow();

    /*request show url wrong dialog*/
    void requestShowUrlWrongDialog(const DUrl& url);

    /*request show PropertyDialog*/
    void requestShowOpenWithDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowPropertyDialog(const DFMUrlListBaseEvent &event);

    /*request show PropertyDialog*/
    void requestShowShareOptionsInPropertyDialog(const DFMUrlListBaseEvent &event);

    /*request show TrashPropertyDialog*/
    void requestShowTrashPropertyDialog(const DFMEvent &event);

    /*request show ComputerPropertyDialog*/
    void requestShowComputerPropertyDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowDevicePropertyDialog(const DFMEvent &event);

    /* request select file view item*/
    void requestSelectFile(const DFMUrlListBaseEvent &event);

    /* request select file view item and rename*/
    void requestSelectRenameFile(const DFMUrlBaseEvent &event);

    /*request update mime cache*/
    void requestUpdateMimeAppsCache();

    /*close last active window */
    void aboutToCloseLastActivedWindow(int winId);

    /*sort role changed*/
    void requestCacheSortState();

    /* error dialog from disk listener*/
    void showDiskErrorDialog(const QString &id, const QString &errorText);

    /* status bar items count */
    void statusBarItemsCounted(const DFMEvent &event, int number);
    void statusBarItemsSelected(const DFMEvent &event, int number);

    /* request of renaming bookmark*/
    void requestBookmarkRename(const DFMEvent &event);

    /*book mark rename*/
    void bookmarkRenamed(const QString &newname, const DFMUrlBaseEvent &event);

    /*focus back to DFileView*/
    void requestFoucsOnFileView(quint64 winId);

    /*ctrl + F*/
    void requestSearchCtrlF(quint64 winId);

    /*ctrl + L*/
    void requestSearchCtrlL(quint64 winId);

    /* about dialog */
    void showAboutDialog(quint64 winId);

    /*request cache samba login data*/
    void requsetCacheLoginData(const QJsonObject& obj);

    /*quit application if taskdialog close*/
    void requestQuitApplication();

    /*request fresh file view*/
    void requestFreshFileView(quint64 winId);
    void requestFreshAllFileView();

    /*request abort copy/move job*/
    void requestAbortJob(const DUrl& url);

    /*request start file job task update timer*/
    void requestStartUpdateJobTimer();

    void requestStopUpdateJobTimer();

    /*icon size changed by setting dialog*/
    void requestChangeIconSizeBySizeIndex(const int& sizeIndex);

    /*show hidden by setting dialog changed*/
    void showHiddenOnViewChanged();

    /*trash state changed when trash is empty or is filled*/
    void trashStateChanged();

    /*show file preview dialog*/
    void requestShowFilePreviewDialog(const QSharedPointer<DFMUrlListBaseEvent> &event);

    /*show 4G warning dialog if file copy to removal devicve which type is fat32*/
    void show4GFat32Dialog();

#ifdef SW_LABEL
    /*copy/move/delete fail job show */
    void jobFailed(int nRet, const QString &jobType, const QString& srcFileName);
#endif
};

#endif // FILESIGNALMANAGER_H
