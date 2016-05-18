#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include "global.h"

#include <QObject>
#include <QDir>
#include <QDebug>

class FileInfo;
class FMEvent;
class AbstractFileInfo;
class DeviceInfo;

class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0) :
        QObject(parent){}

signals:
    /* change currentUrl*/
    void requestChangeCurrentUrl(const FMEvent &event);

    /* current display url changed*/
    void currentUrlChanged(const FMEvent &event);

    /* file icon*/
    void requestIcon(const DUrl &url) const;
    void iconChanged(const DUrl &url, const QIcon &icon) const;

    /* refresh folder*/
    void refreshFolder(const DUrl &url) const;

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

    /*conflictTimer in move/copy job  stop when conflict dialog showed*/
    void conflictTimerStoped(const QMap<QString, QString>& jobDetail);

    /*conflictTimer in move/copy job restart when conflict dialog showed*/
    void conflictTimerReStarted(const QMap<QString, QString>& jobDetail);

    /*copy/move job conflict response confirmed from ui*/
    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

    /*request rename to DFileView*/
    void requestRename(const FMEvent &event);

    /* view select all */
    void requestViewSelectAll(int windowId);

    /* bookmark remove */
    void requestBookmarkRemove(const FMEvent &event);

    /* bookmark add */
    void requestBookmarkAdd(const QString &name, const FMEvent &event);

    /* bookmark move */
    void requestBookmarkMove(int from, int to, const FMEvent &event);

    /*requst open new window by given url*/
    void requestOpenNewWindowByUrl(const DUrl& url, bool isAlwaysOpen);

    /* device mounted */
    void deviceMounted(int i);

    /* device added */
    void deviceAdded(DeviceInfo &deviceInfos);

    /* device removed */
    void deviceRemoved(DeviceInfo &deviceInfos);

    /*request active window*/
    void requestActiveWindow();

    /*request show url wrong dialog*/
    void requestShowUrlWrongDialog(const DUrl& url);

    /*request show PropertyDialog*/
    void requestShowOpenWithDialog(const FMEvent &event);

    /*request show PropertyDialog*/
    void requestShowPropertyDialog(const FMEvent &event);

    /* request select file view item*/
    void requestSelectFile(const FMEvent &event);

    /* request restore trash file*/
    void requestRestoreTrashFile(const DUrlList &urlList, const FMEvent &event);

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
};

#endif // FILESIGNALMANAGER_H
