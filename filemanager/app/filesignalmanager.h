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
    void requestIcon(const QString &url) const;
    void iconChanged(const QString &url, const QIcon &icon) const;

    /* refresh folder*/
    void refreshFolder(const QString &url) const;

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

    /* view sort */
    void requestViewSort(int windowId, Global::SortRole role);

    /* view select all */
    void requestViewSelectAll(int windowId);

    /* view select all */
    void requestBookmarkRemove(const QString &url);

    /*requst open new window by given url*/
    void requestOpenNewWindowByUrl(const QString& url);

    /* device mounted */
    void deviceMounted(int i);

    /* device added */
    void deviceAdded(DeviceInfo &deviceInfos);

    /* device removed */
    void deviceRemoved(DeviceInfo &deviceInfos);

    /*request active window*/
    void requestActiveWindow();

    /*request show url wrong dialog*/
    void requestShowUrlWrongDialog(const QString& url);

    /*request show PropertyDialog*/
    void requestShowOpenWithDialog(const QString& url);

    /*request show PropertyDialog*/
    void requestShowPropertyDialog(const QString& url);

    /* request select file view item*/
    void requestSelectFile(const FMEvent &event);
};

#endif // FILESIGNALMANAGER_H
