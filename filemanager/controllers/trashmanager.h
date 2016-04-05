#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "basemanager.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QFileInfo>

class FileInfo;

class TrashManager  : public BaseManager
{
    Q_OBJECT
public:
    TrashManager(QObject *parent = 0);
    void load();
    void save();
private:
    QList<FileInfo*> m_fileInfos;
signals:
    void updates(const QString &directory, const QList<FileInfo*> &infoList);

public slots:
    void fetchFileInformation(const QString &url,
                              int filter = int(QDir::AllEntries | QDir::NoDotDot));
    void trashUpdated();

};

#endif // TRASHMANAGER_H
