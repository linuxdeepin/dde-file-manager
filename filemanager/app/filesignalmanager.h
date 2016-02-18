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
    void getChildren(const QUrl &url) const;
    void getChildrenFinished(const QUrl &url, const FileItemInfoList &list) const;

    void getIcon(const QUrl &url) const;
    void getIconFinished(const QUrl &url, const QIcon &icon) const;
};

#endif // FILESIGNALMANAGER_H
