#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include <QObject>
#include <QDir>
#include <QDebug>

class FileInfo;
class FMEvent;

class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0) :
        QObject(parent){}

signals:
    /// change currentUrl
    void requestChangeCurrentUrl(const FMEvent &event);
    /// current display url changed
    void currentUrlChanged(const FMEvent &event);

    /// in folder files
    void requestChildren(const QString &url, int filter = int(QDir::AllEntries | QDir::NoDotDot)) const;
    void childrenChanged(const QString &url, const QList<FileInfo*> &list) const;

    /// file icon
    void requestIcon(const QString &url) const;
    void iconChanged(const QString &url, const QIcon &icon) const;

    /// refresh folder
    void refreshFolder(const QString &url) const;

    /// open file signal
    void fileOpened(const QString &url) const;

    /*dbus to copy job dialog */
    void jobAdded(const QMap<QString, QString>& jobDetail);
    void jobRemoved(const QMap<QString, QString>& jobDetail);

    void jobDataUpdated(const QMap<QString, QString>& jobDetail,
                           const QMap<QString, QString>& data);

    void abortTask(const QMap<QString, QString>& jobDetail);

//    /*copy/move job conflict dialog show */
//    void conflictDialogShowed(const QMap<QString, QString>& jobDetail);
//    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

//    void conflictTimerStoped(const QMap<QString, QString>& jobDetail);
//    void conflictTimerReStarted(const QMap<QString, QString>& jobDetail);
};

#endif // FILESIGNALMANAGER_H
