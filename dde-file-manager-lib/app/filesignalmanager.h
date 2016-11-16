#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include <QObject>

class DFileInfo;
class DFMEvent;
class DAbstractFileInfo;
class DeviceInfo;
class DUrl;

typedef QList<DUrl> DUrlList;

class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0) :
        QObject(parent){}

signals:
    /* change currentUrl*/
    void requestChangeCurrentUrl(const DFMEvent &event);

    /* emit usershare status changed*/
    void userShareCountChanged(const int count);

    /* request open in new tab*/
    void requestOpenInNewTab(const DFMEvent &event);

    /* request close current tab*/
    void requestCloseCurrentTab(const DFMEvent &event);

    /* request fetch network nodes*/

    void requestFetchNetworks(const DFMEvent &event);

    /*request fetch network nodes successfully*/
    void fetchNetworksSuccessed(const DFMEvent &event);

    /*request mount smb*/
    void requestSMBMount(const DFMEvent &event);

    /*request choose smb mounted file*/
    void requestChooseSmbMountedFile(const DFMEvent &event);

    /* current display url changed*/
    void currentUrlChanged(const DFMEvent &event);

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
    void requestRename(const DFMEvent &event);

    /* view select all */
    void requestViewSelectAll(int windowId);

    /* bookmark remove */
    void requestBookmarkRemove(const DFMEvent &event);

    /* bookmark add */
    void requestBookmarkAdd(const QString &name, const DFMEvent &event);

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
    void requestShowPropertyDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowShareOptionsInPropertyDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowTrashPropertyDialog(const DFMEvent &event);

    /*request show PropertyDialog*/
    void requestShowDevicePropertyDialog(const DFMEvent &event);

    /* request select file view item*/
    void requestSelectFile(const DFMEvent &event);

    /* request select file view item and rename*/
    void requestSelectRenameFile(const DFMEvent &event);

    /* request restore trash file*/
    void requestRestoreTrashFile(const DUrlList &urlList, const DFMEvent &event);

    /* request restore trash file*/
    void requestRestoreAllTrashFile(const DFMEvent &event);

    /*request update mime cache*/
    void requestUpdateMimeAppsCache();

    /*request clear recent*/
    void requestClearRecent();
    /*request remove recent file*/
    void requestRecentFileRemove(const DUrlList &urlList);

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
    void bookmarkRenamed(const QString &newname, const DFMEvent &event);

    /*focus back to DFileView*/
    void requestFoucsOnFileView(const DFMEvent &event);

    /*ctrl + F*/
    void requestSearchCtrlF(const DFMEvent &event);

    /*ctrl + L*/
    void requestSearchCtrlL(const DFMEvent &event);

    /*alt + left*/
    void requestBack(const DFMEvent &event);

    /*alt + right*/
    void requestForward(const DFMEvent &event);

    /* about dialog */
    void showAboutDialog(const DFMEvent &event);

    /*loading indcator show*/
    void loadingIndicatorShowed(const DFMEvent &event, bool loading);

    /*request cache samba login data*/
    void requsetCacheLoginData(const QJsonObject& obj);

    /*quit application if taskdialog close*/
    void requestQuitApplication();

    /*request fresh file view*/
    void requestFreshFileView(const DFMEvent &event);

    /*request abort copy/move job*/
    void requestAbortJob(const DUrl& url);

    /*request start file job task update timer*/
    void requestStartUpdateJobTimer();

    void requestStopUpdateJobTimer();
};

#endif // FILESIGNALMANAGER_H
