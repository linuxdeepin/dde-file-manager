#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include "dbusinterface/dbustype.h"

#include <QObject>
#include <QDebug>

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
    void requestChildren(const QString &url) const;
    void childrenChanged(const QString &url, const FileItemInfoList &list) const;

    /// file icon
    void requestIcon(const QString &url) const;
    void iconChanged(const QString &url, const QIcon &icon) const;
};

#endif // FILESIGNALMANAGER_H
