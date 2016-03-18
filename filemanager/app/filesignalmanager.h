#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include <QObject>
#include <QDir>
#include <QDebug>

class FileInfo;

class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0) :
        QObject(parent){}

signals:
    /// current display url changed
    void currentUrlChanged(const QString &url);

    /// in folder files
    void requestChildren(const QString &url, int filter = int(QDir::AllEntries | QDir::NoDotDot)) const;
    void childrenChanged(const QString &url, const QList<FileInfo*> &list) const;

    /// file icon
    void requestIcon(const QString &url) const;
    void iconChanged(const QString &url, const QIcon &icon) const;

    /// refresh folder
    void refreshFolder(const QString &url) const;
};

#endif // FILESIGNALMANAGER_H
