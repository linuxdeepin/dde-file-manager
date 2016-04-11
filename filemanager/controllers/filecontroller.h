#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include "abstractfilecontroller.h"

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;
class AbstractFileInfo;
class FileMonitor;

class FileController : public AbstractFileController
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);

    AbstractFileInfo *createFileInfo(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfo*> getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const QString &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool renameFile(const QString &oldUrl, const QString &newUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool addUrlMonitor(const QString &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const QString &url, bool &accepted) const Q_DECL_OVERRIDE;

private slots:
    void onFileCreated(const QString &filePath);
    void onFileRemove(const QString &filePath);
    void onFileRename(const QString &oldName, const QString &newName);

private:
    FileMonitor *fileMonitor;
};

#endif // FILECONTROLLER_H
