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
    explicit TrashManager(QObject *parent = 0);

    AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfo*> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool addUrlMonitor(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool restoreTrashFile(const DUrl &fileUrl) const;

private slots:
    void onFileCreated(const QString &filePath) const;
    void onFileRemove(const QString &filePath) const;

private:
    mutable FileMonitor *fileMonitor = Q_NULLPTR;
};

#endif // TRASHMANAGER_H
