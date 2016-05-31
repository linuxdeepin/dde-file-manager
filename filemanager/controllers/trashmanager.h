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

    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool addUrlMonitor(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DUrlList &urlList, const FMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool restoreTrashFile(const DUrlList &fileUrl, const FMEvent &event) const;
    bool restoreAllTrashFile(const FMEvent &event);

    static bool isEmpty();

private slots:
    void onFileCreated(const QString &filePath) const;
    void onFileRemove(const QString &filePath) const;

private:
    mutable FileMonitor *fileMonitor = Q_NULLPTR;
};

#endif // TRASHMANAGER_H
