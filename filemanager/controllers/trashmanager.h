#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "abstractfilecontroller.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QFileInfo>

class AbstractFileInfo;
class FileMonitor;

class TrashManager : public AbstractFileController
{
    Q_OBJECT

public:
    TrashManager(QObject *parent = 0);

    AbstractFileInfo *createFileInfo(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfo*> getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool addUrlMonitor(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const QString &url, bool &accepted) const Q_DECL_OVERRIDE;

private slots:
    void onFileCreated(const QString &filePath);
    void onFileRemove(const QString &filePath);
    void onFileRename(const QString &oldName, const QString &newName);

private:
    mutable FileMonitor *fileMonitor = Q_NULLPTR;
};

#endif // TRASHMANAGER_H
