#ifndef FILESIGNALMANAGER_H
#define FILESIGNALMANAGER_H

#include "dbusinterface/dbustype.h"

#include <QObject>
#include <QUrl>
#include <QDebug>

class FileSignalManager : public QObject
{
    Q_OBJECT

public:
    explicit FileSignalManager(QObject *parent = 0) :
        QObject(parent){}

signals:
    /// current display url changed
    void currentUrlChanged(const QUrl &url);

    /// in folder files
    void requestChildren(const QUrl &url) const;
    void childrenChanged(const QUrl &url, const FileItemInfoList &list) const;

    /// file icon
    void requestIcon(const QUrl &url) const;
    void iconChanged(const QUrl &url, const QIcon &icon) const;
};

#endif // FILESIGNALMANAGER_H
